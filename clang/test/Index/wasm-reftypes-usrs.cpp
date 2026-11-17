// REQUIRES: webassembly-registered-target
// RUN: c-index-test -test-load-source-usrs all -target wasm32 -mgc %s | FileCheck %s

// The WebAssembly reference types are distinct, so overloads that differ only
// in which one they take need distinct USRs.
void overloaded(__externref_t);
void overloaded(__non_null_externref_t);

// CHECK: wasm-reftypes-usrs.cpp c:@F@overloaded#@BT@__externref_t# Extent=[6:1 - 6:31]
// CHECK: wasm-reftypes-usrs.cpp c:@F@overloaded#@BT@__non_null_externref_t# Extent=[7:1 - 7:40]
