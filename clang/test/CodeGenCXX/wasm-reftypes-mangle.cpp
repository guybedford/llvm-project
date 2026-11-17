// REQUIRES: webassembly-registered-target
// RUN: %clang_cc1 %s -triple wasm32-unknown-unknown -target-feature +reference-types -emit-llvm -o - -std=c++11 | FileCheck %s
// RUN: %clang_cc1 %s -triple wasm64-unknown-unknown -target-feature +reference-types -emit-llvm -o - -std=c++11 | FileCheck %s
// RUN: %clang_cc1 %s -triple wasm32-unknown-unknown -target-feature +reference-types -target-feature +gc -DGC -emit-llvm -o - -std=c++11 | FileCheck %s --check-prefixes=CHECK,GC
// RUN: %clang_cc1 %s -triple wasm64-unknown-unknown -target-feature +reference-types -target-feature +gc -DGC -emit-llvm -o - -std=c++11 | FileCheck %s --check-prefixes=CHECK,GC

// Test that funcref can be used in C++ without crashing during codegen.
// See https://github.com/llvm/llvm-project/issues/176154
typedef void (*__funcref funcref_t)();

// Global funcref variables - test that codegen doesn't crash.
// CHECK-DAG: @fptr = global target("wasm.funcref") zeroinitializer
funcref_t fptr;

// CHECK-DAG: @fpt2 = global target("wasm.funcref") zeroinitializer
void (*__funcref fpt2)();

// CHECK-DAG: _Z2f1u11externref_t
void f1(__externref_t) {}

// CHECK-DAG: _Z2f2PU4AS20FvvE
void f2(funcref_t) {}

#ifdef GC
// The non-nullable externref is a distinct type with its own mangling.
// GC-DAG: _Z2f1u20non_null_externref_t
void f1(__non_null_externref_t) {}

// GC-DAG: _Z2f3u11externref_tu20non_null_externref_t
void f3(__externref_t, __non_null_externref_t) {}
#endif
