//===----- SemaWasm.h ------ Wasm target-specific routines ----*- C++ -*---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file declares semantic analysis functions specific to Wasm.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_SEMAWASM_H
#define LLVM_CLANG_SEMA_SEMAWASM_H

#include "clang/AST/ASTFwd.h"
#include "clang/Sema/SemaBase.h"

namespace clang {
class ParsedAttr;
class TargetInfo;

class SemaWasm : public SemaBase {
public:
  SemaWasm(Sema &S);

  bool CheckWebAssemblyBuiltinFunctionCall(const TargetInfo &TI,
                                           unsigned BuiltinID,
                                           CallExpr *TheCall);

  /// If one operand of a conditional expression is a non-nullable externref
  /// and the other a nullable externref, converts the former to the latter
  /// and returns the nullable type. Otherwise returns a null type. Both
  /// operands must already be prvalues.
  QualType CheckConditionalExternrefOperands(ExprResult &LHS, ExprResult &RHS);

  bool BuiltinWasmRefNullExtern(CallExpr *TheCall);
  bool BuiltinWasmRefIsNullExtern(CallExpr *TheCall);
  bool BuiltinWasmRefAsNonNullExtern(CallExpr *TheCall);
  bool BuiltinWasmRefNullFunc(CallExpr *TheCall);
  bool BuiltinWasmTableGet(CallExpr *TheCall);
  bool BuiltinWasmTableSet(CallExpr *TheCall);
  bool BuiltinWasmTableSize(CallExpr *TheCall);
  bool BuiltinWasmTableGrow(CallExpr *TheCall);
  bool BuiltinWasmTableFill(CallExpr *TheCall);
  bool BuiltinWasmTableCopy(CallExpr *TheCall);
  bool BuiltinWasmTestFunctionPointerSignature(const TargetInfo &TI,
                                               CallExpr *TheCall);

  WebAssemblyImportNameAttr *
  mergeImportNameAttr(Decl *D, const WebAssemblyImportNameAttr &AL);
  WebAssemblyImportModuleAttr *
  mergeImportModuleAttr(Decl *D, const WebAssemblyImportModuleAttr &AL);
  WebAssemblyExportNameAttr *
  mergeExportNameAttr(Decl *D, const WebAssemblyExportNameAttr &AL);

  void handleWebAssemblyExportNameAttr(Decl *D, const ParsedAttr &AL);
  void handleWebAssemblyImportModuleAttr(Decl *D, const ParsedAttr &AL);
  void handleWebAssemblyImportNameAttr(Decl *D, const ParsedAttr &AL);
};
} // namespace clang

#endif // LLVM_CLANG_SEMA_SEMAWASM_H
