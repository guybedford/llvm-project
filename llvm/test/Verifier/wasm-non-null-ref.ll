; RUN: split-file %s %t
; RUN: not llvm-as < %t/intrinsic-arg.ll -disable-output 2>&1 | FileCheck --check-prefix=CHECK-ARG %s
; RUN: not llvm-as < %t/intrinsic-ret.ll -disable-output 2>&1 | FileCheck --check-prefix=CHECK-RET %s
; RUN: not llvm-as < %t/zeroinit.ll -disable-output 2>&1 | FileCheck --check-prefix=CHECK-ZEROINIT %s
; RUN: not llvm-as < %t/global.ll -disable-output 2>&1 | FileCheck --check-prefix=CHECK-GLOBAL %s
; RUN: llvm-as < %t/valid.ll -disable-output

; The non-nullable (ref extern) is a distinct type from externref with no
; default value.

;--- intrinsic-arg.ll
declare i32 @llvm.wasm.ref.is_null.extern(target("wasm.externref.nonnull"))
; CHECK-ARG: intrinsic argument 0 type expected wasm.externref, but got target("wasm.externref.nonnull")

;--- intrinsic-ret.ll
declare target("wasm.externref") @llvm.wasm.ref.as_non_null.extern(target("wasm.externref"))
; CHECK-RET: intrinsic return type expected wasm.externref.nonnull, but got target("wasm.externref")

;--- zeroinit.ll
define target("wasm.externref.nonnull") @f() {
  ret target("wasm.externref.nonnull") zeroinitializer
}
; CHECK-ZEROINIT: invalid type for null constant

;--- global.ll
@g = global target("wasm.externref.nonnull") poison
; CHECK-GLOBAL: Global @g has illegal target extension type

;--- valid.ll
declare target("wasm.externref.nonnull") @llvm.wasm.ref.as_non_null.extern(target("wasm.externref"))
declare target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull"))

define target("wasm.externref") @f(target("wasm.externref") %r) {
  %slot = alloca target("wasm.externref.nonnull")
  %nn = call target("wasm.externref.nonnull") @llvm.wasm.ref.as_non_null.extern(target("wasm.externref") %r)
  store target("wasm.externref.nonnull") %nn, ptr %slot
  %v = load target("wasm.externref.nonnull"), ptr %slot
  %n = call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") %v)
  ret target("wasm.externref") %n
}
