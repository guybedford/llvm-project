// RUN: %clang_cc1 -triple wasm32-unknown-unknown -target-feature +reference-types -target-feature +gc -O0 -emit-llvm %s -o - | FileCheck %s --check-prefixes=IR,IR-O0
// RUN: %clang_cc1 -triple wasm32-unknown-unknown -target-feature +reference-types -target-feature +gc -O2 -emit-llvm %s -o - | FileCheck %s --check-prefixes=IR,IR-O2

// The JS string builtins return a non-nullable (ref extern), which has to be
// part of the import's signature for the module to instantiate.
// See https://github.com/llvm/llvm-project/issues/136594.

__non_null_externref_t concat(__externref_t, __externref_t)
    __attribute__((import_module("wasm:js-string"), import_name("concat")));
int length(__externref_t)
    __attribute__((import_module("wasm:js-string"), import_name("length")));

void consume(__externref_t);
void consume_non_null(__non_null_externref_t);

// IR-LABEL: define target("wasm.externref.nonnull") @concat3(
// IR-SAME:    target("wasm.externref") %a, target("wasm.externref") %b, target("wasm.externref") %c)
// IR:         [[INNER:%.*]] = {{(tail )?}}call target("wasm.externref.nonnull") @concat(target("wasm.externref") {{%.*}}, target("wasm.externref") {{%.*}})
// IR-NEXT:    [[NULLABLE:%.*]] = {{(tail )?}}call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") [[INNER]])
// IR:         [[OUTER:%.*]] = {{(tail )?}}call target("wasm.externref.nonnull") @concat(target("wasm.externref") [[NULLABLE]], target("wasm.externref") {{%.*}})
// IR-NEXT:    ret target("wasm.externref.nonnull") [[OUTER]]
__non_null_externref_t concat3(__externref_t a, __externref_t b,
                               __externref_t c) {
  return concat(concat(a, b), c);
}

// IR:       declare target("wasm.externref.nonnull") @concat(target("wasm.externref"), target("wasm.externref")) {{.*}}#[[CONCAT_ATTRS:[0-9]+]]

// IR-LABEL: define i32 @concat_length(
// IR:         [[STR:%.*]] = {{(tail )?}}call target("wasm.externref.nonnull") @concat(
// IR-NEXT:    [[NULLABLE:%.*]] = {{(tail )?}}call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") [[STR]])
// IR-NEXT:    [[LEN:%.*]] = {{(tail )?}}call i32 @length(target("wasm.externref") [[NULLABLE]])
// IR-NEXT:    ret i32 [[LEN]]
int concat_length(__externref_t a, __externref_t b) {
  return length(concat(a, b));
}

// IR-LABEL: define void @concat_and_consume(
// IR-O0:      [[STR_ADDR:%.*]] = alloca target("wasm.externref.nonnull"), align 1
// IR:         [[STR:%.*]] = {{(tail )?}}call target("wasm.externref.nonnull") @concat(
// IR-O0-NEXT: store target("wasm.externref.nonnull") [[STR]], ptr [[STR_ADDR]], align 1
// IR-O0-NEXT: [[LOAD:%.*]] = load target("wasm.externref.nonnull"), ptr [[STR_ADDR]], align 1
// IR-O0-NEXT: [[NULLABLE:%.*]] = call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") [[LOAD]])
// IR-O0-NEXT: call void @consume(target("wasm.externref") [[NULLABLE]])
// IR-O0-NEXT: [[RELOAD:%.*]] = load target("wasm.externref.nonnull"), ptr [[STR_ADDR]], align 1
// IR-O0-NEXT: call void @consume_non_null(target("wasm.externref.nonnull") [[RELOAD]])
// IR-O2-NEXT: [[NULLABLE:%.*]] = tail call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") [[STR]])
// IR-O2-NEXT: tail call void @consume(target("wasm.externref") [[NULLABLE]])
// IR-O2-NEXT: tail call void @consume_non_null(target("wasm.externref.nonnull") [[STR]])
// IR-NEXT:    ret void
void concat_and_consume(__externref_t a, __externref_t b) {
  __non_null_externref_t str = concat(a, b);
  consume(str);
  consume_non_null(str);
}

// IR-LABEL: define target("wasm.externref.nonnull") @concat_checked(
// IR:         [[NON_NULL:%.*]] = {{(tail )?}}call target("wasm.externref.nonnull") @llvm.wasm.ref.as_non_null.extern(target("wasm.externref") {{%.*}})
// IR-NEXT:    [[NULLABLE:%.*]] = {{(tail )?}}call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") [[NON_NULL]])
// IR:         [[STR:%.*]] = {{(tail )?}}call target("wasm.externref.nonnull") @concat(target("wasm.externref") [[NULLABLE]], target("wasm.externref") {{%.*}})
// IR-NEXT:    ret target("wasm.externref.nonnull") [[STR]]
__non_null_externref_t concat_checked(__externref_t a, __externref_t b) {
  return concat(__builtin_wasm_ref_as_non_null_extern(a), b);
}

// IR: attributes #[[CONCAT_ATTRS]] = { {{.*}}"wasm-import-module"="wasm:js-string" "wasm-import-name"="concat" }
