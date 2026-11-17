// REQUIRES: webassembly-registered-target
// RUN: %clang_cc1 -triple wasm32-unknown-unknown -target-feature +reference-types -target-feature +gc -emit-pch -o %t %s
// RUN: %clang_cc1 -triple wasm32-unknown-unknown -target-feature +reference-types -target-feature +gc -include-pch %t -emit-llvm -o - %s | FileCheck %s

#ifndef HEADER
#define HEADER

__non_null_externref_t concat(__externref_t, __externref_t);

static inline __externref_t widen(__non_null_externref_t ref) { return ref; }

static inline __non_null_externref_t narrow(__externref_t ref) {
  return __builtin_wasm_ref_as_non_null_extern(ref);
}

#else

// CHECK-LABEL: define {{.*}}target("wasm.externref") @test(
// CHECK: call target("wasm.externref.nonnull") @narrow(
// CHECK: call target("wasm.externref.nonnull") @concat(
// CHECK: call target("wasm.externref") @widen(
// CHECK-LABEL: define internal target("wasm.externref") @widen(
// CHECK: call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(
// CHECK-LABEL: define internal target("wasm.externref.nonnull") @narrow(
// CHECK: call target("wasm.externref.nonnull") @llvm.wasm.ref.as_non_null.extern(
__externref_t test(__externref_t a, __externref_t b) {
  return widen(concat(narrow(a), b));
}

#endif
