//=- WebAssemblyMachineFunctionInfo.cpp - WebAssembly Machine Function Info -=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file implements WebAssembly-specific per-machine-function
/// information.
///
//===----------------------------------------------------------------------===//

#include "WebAssemblyMachineFunctionInfo.h"
#include "Utils/WebAssemblyTypeUtilities.h"
#include "WebAssemblyISelLowering.h"
#include "WebAssemblySubtarget.h"
#include "WebAssemblyUtilities.h"
#include "llvm/CodeGen/Analysis.h"
#include "llvm/IR/Module.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

WebAssemblyFunctionInfo::~WebAssemblyFunctionInfo() = default; // anchor.

MachineFunctionInfo *WebAssemblyFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<WebAssemblyFunctionInfo>(*this);
}

void WebAssemblyFunctionInfo::initWARegs(MachineRegisterInfo &MRI) {
  assert(WARegs.empty());
  unsigned Reg = WebAssembly::UnusedReg;
  WARegs.resize(MRI.getNumVirtRegs(), Reg);
}

void llvm::computeLegalValueVTs(const WebAssemblyTargetLowering &TLI,
                                LLVMContext &Ctx, const DataLayout &DL,
                                Type *Ty, SmallVectorImpl<MVT> &ValueVTs) {
  SmallVector<EVT, 4> VTs;
  ComputeValueVTs(TLI, DL, Ty, VTs);

  for (EVT VT : VTs) {
    unsigned NumRegs = TLI.getNumRegisters(Ctx, VT);
    MVT RegisterVT = TLI.getRegisterType(Ctx, VT);
    for (unsigned I = 0; I != NumRegs; ++I)
      ValueVTs.push_back(RegisterVT);
  }
}

void llvm::computeLegalValueVTs(const Function &F, const TargetMachine &TM,
                                Type *Ty, SmallVectorImpl<MVT> &ValueVTs) {
  const DataLayout &DL(F.getDataLayout());
  const WebAssemblyTargetLowering &TLI =
      *TM.getSubtarget<WebAssemblySubtarget>(F).getTargetLowering();
  computeLegalValueVTs(TLI, F.getContext(), DL, Ty, ValueVTs);
}

void llvm::computeLegalValTypes(const Function &F, const TargetMachine &TM,
                                Type *Ty,
                                SmallVectorImpl<wasm::ValType> &ValTypes) {
  SmallVector<Type *, 4> Types;
  ComputeValueTypes(F.getDataLayout(), Ty, Types);
  for (Type *ValueTy : Types) {
    SmallVector<MVT, 1> VTs;
    computeLegalValueVTs(F, TM, ValueTy, VTs);
    for (MVT VT : VTs) {
      wasm::ValType ValTy = WebAssembly::toValType(VT);
      ValTypes.push_back(WebAssembly::isWebAssemblyNonNullRefType(ValueTy)
                             ? ValTy.asNonNullable()
                             : ValTy);
    }
  }
}

// Legalize appends the types a given IR type is passed as.
template <typename T, typename LegalizeT>
static void
computeSignatureImpl(const FunctionType *Ty, const Function *TargetFunc,
                     const Function &ContextFunc, const TargetMachine &TM,
                     T PtrTy, LegalizeT Legalize, SmallVectorImpl<T> &Params,
                     SmallVectorImpl<T> &Results) {
  Legalize(Ty->getReturnType(), Results);

  if (!WebAssembly::canLowerReturn(
          Results.size(),
          &TM.getSubtarget<WebAssemblySubtarget>(ContextFunc))) {
    // WebAssembly can't lower returns of multiple values without demoting to
    // sret unless multivalue is enabled (see
    // WebAssemblyTargetLowering::CanLowerReturn). So replace multiple return
    // values with a pointer parameter.
    Results.clear();
    Params.push_back(PtrTy);
  }

  std::optional<size_t> SwiftAsyncIdx;
  for (auto [I, Param] : enumerate(Ty->params())) {
    if (TargetFunc && TargetFunc->hasParamAttribute(I, Attribute::SwiftAsync))
      SwiftAsyncIdx = Params.size();
    Legalize(Param, Params);
  }
  // ABI contract: the async context must always be available in local 0.
  if (SwiftAsyncIdx)
    WebAssembly::moveToFront(Params, *SwiftAsyncIdx);
  if (Ty->isVarArg())
    Params.push_back(PtrTy);

  // For swiftcc and swifttailcc, emit additional swiftself, swifterror, and
  // (for swifttailcc) swiftasync parameters if there aren't. These additional
  // parameters are also passed for caller. They are necessary to match callee
  // and caller signature for indirect call.

  if (TargetFunc && (TargetFunc->getCallingConv() == CallingConv::Swift ||
                     TargetFunc->getCallingConv() == CallingConv::SwiftTail)) {
    bool HasSwiftErrorArg = false;
    bool HasSwiftSelfArg = false;
    bool HasSwiftAsyncArg = false;
    for (const auto &Arg : TargetFunc->args()) {
      HasSwiftErrorArg |= Arg.hasAttribute(Attribute::SwiftError);
      HasSwiftSelfArg |= Arg.hasAttribute(Attribute::SwiftSelf);
      HasSwiftAsyncArg |= Arg.hasAttribute(Attribute::SwiftAsync);
    }
    if (!HasSwiftSelfArg)
      Params.push_back(PtrTy);
    if (!HasSwiftErrorArg)
      Params.push_back(PtrTy);
    if (TargetFunc->getCallingConv() == CallingConv::SwiftTail &&
        !HasSwiftAsyncArg)
      Params.insert(Params.begin(), PtrTy);
  }
}

void llvm::computeSignatureVTs(const FunctionType *Ty,
                               const Function *TargetFunc,
                               const Function &ContextFunc,
                               const TargetMachine &TM,
                               SmallVectorImpl<MVT> &Params,
                               SmallVectorImpl<MVT> &Results) {
  const DataLayout &DL = ContextFunc.getDataLayout();
  MVT PtrVT = MVT::getIntegerVT(DL.getPointerSizeInBits());
  computeSignatureImpl(
      Ty, TargetFunc, ContextFunc, TM, PtrVT,
      [&](Type *T, SmallVectorImpl<MVT> &VTs) {
        computeLegalValueVTs(ContextFunc, TM, T, VTs);
      },
      Params, Results);
}

void llvm::computeSignature(const FunctionType *Ty, const Function *TargetFunc,
                            const Function &ContextFunc,
                            const TargetMachine &TM,
                            SmallVectorImpl<wasm::ValType> &Params,
                            SmallVectorImpl<wasm::ValType> &Results) {
  const DataLayout &DL = ContextFunc.getDataLayout();
  wasm::ValType PtrTy =
      WebAssembly::toValType(MVT::getIntegerVT(DL.getPointerSizeInBits()));
  computeSignatureImpl(
      Ty, TargetFunc, ContextFunc, TM, PtrTy,
      [&](Type *T, SmallVectorImpl<wasm::ValType> &ValTypes) {
        computeLegalValTypes(ContextFunc, TM, T, ValTypes);
      },
      Params, Results);
}

wasm::WasmSignature *llvm::computeSignature(MCContext &Ctx,
                                            const FunctionType *Ty,
                                            const Function *TargetFunc,
                                            const Function &ContextFunc,
                                            const TargetMachine &TM) {
  auto *Sig = Ctx.createWasmSignature();
  computeSignature(Ty, TargetFunc, ContextFunc, TM, Sig->Params, Sig->Returns);
  return Sig;
}

void llvm::valTypesFromMVTs(ArrayRef<MVT> In,
                            SmallVectorImpl<wasm::ValType> &Out) {
  for (MVT Ty : In)
    Out.push_back(WebAssembly::toValType(Ty));
}

yaml::WebAssemblyFunctionInfo::WebAssemblyFunctionInfo(
    const llvm::MachineFunction &MF, const llvm::WebAssemblyFunctionInfo &MFI)
    : CFGStackified(MFI.isCFGStackified()) {
  for (auto VT : MFI.getParams())
    Params.push_back(EVT(VT).getEVTString());
  for (auto VT : MFI.getResults())
    Results.push_back(EVT(VT).getEVTString());
}

void yaml::WebAssemblyFunctionInfo::mappingImpl(yaml::IO &YamlIO) {
  MappingTraits<WebAssemblyFunctionInfo>::mapping(YamlIO, *this);
}

void WebAssemblyFunctionInfo::initializeBaseYamlFields(
    MachineFunction &MF, const yaml::WebAssemblyFunctionInfo &YamlMFI) {
  CFGStackified = YamlMFI.CFGStackified;
  for (auto VT : YamlMFI.Params)
    addParam(WebAssembly::parseMVT(VT.Value));
  for (auto VT : YamlMFI.Results)
    addResult(WebAssembly::parseMVT(VT.Value));
}
