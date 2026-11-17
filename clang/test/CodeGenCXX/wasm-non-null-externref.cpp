// REQUIRES: webassembly-registered-target
// RUN: %clang_cc1 %s -triple wasm32-unknown-unknown -target-feature +reference-types -target-feature +gc -emit-llvm -o - -std=c++11 | FileCheck %s

namespace std {
class type_info;
}

void both(__externref_t);
void both(__non_null_externref_t);
void only_nullable(__externref_t);

__externref_t get_nullable();
__non_null_externref_t get_non_null();

// CHECK-DAG: @_ZTSu20non_null_externref_t = {{.*}} c"u20non_null_externref_t\00", {{.*}}
// CHECK-DAG: @_ZTIu20non_null_externref_t = {{.*}} @_ZTVN10__cxxabiv123__fundamental_type_infoE, {{.*}} @_ZTSu20non_null_externref_t {{.*}}
auto &non_null_typeinfo = typeid(__non_null_externref_t);

// With both overloads present each argument picks its exact match.
// CHECK-LABEL: define void @_Z9overloadsu11externref_tu20non_null_externref_t(
// CHECK-SAME:    target("wasm.externref") %nullable, target("wasm.externref.nonnull") %non_null)
// CHECK:         [[NULLABLE:%.*]] = load target("wasm.externref"), ptr %nullable.addr
// CHECK-NEXT:    call void @_Z4bothu11externref_t(target("wasm.externref") [[NULLABLE]])
// CHECK-NEXT:    [[NON_NULL:%.*]] = load target("wasm.externref.nonnull"), ptr %non_null.addr
// CHECK-NEXT:    call void @_Z4bothu20non_null_externref_t(target("wasm.externref.nonnull") [[NON_NULL]])
// CHECK-NEXT:    [[CALL:%.*]] = call target("wasm.externref.nonnull") @_Z12get_non_nullv()
// CHECK-NEXT:    call void @_Z4bothu20non_null_externref_t(target("wasm.externref.nonnull") [[CALL]])
// CHECK-NEXT:    ret void
void overloads(__externref_t nullable, __non_null_externref_t non_null) {
  both(nullable);
  both(non_null);
  both(get_non_null());
}

// With only the nullable overload a non-nullable argument is converted.
// CHECK-LABEL: define void @_Z8convertsu20non_null_externref_t(
// CHECK:         [[NON_NULL:%.*]] = load target("wasm.externref.nonnull"), ptr %non_null.addr
// CHECK-NEXT:    [[NULLABLE:%.*]] = call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") [[NON_NULL]])
// CHECK-NEXT:    call void @_Z13only_nullableu11externref_t(target("wasm.externref") [[NULLABLE]])
// CHECK-NEXT:    [[NON_NULL2:%.*]] = load target("wasm.externref.nonnull"), ptr %non_null.addr
// CHECK-NEXT:    [[NULLABLE2:%.*]] = call target("wasm.externref") @llvm.wasm.ref.as_nullable.extern(target("wasm.externref.nonnull") [[NON_NULL2]])
// CHECK-NEXT:    call void @_Z13only_nullableu11externref_t(target("wasm.externref") [[NULLABLE2]])
// CHECK-NEXT:    ret void
void converts(__non_null_externref_t non_null) {
  only_nullable(non_null);
  only_nullable(static_cast<__externref_t>(non_null));
}

template <typename T> T identity(T t) { return t; }

// CHECK-LABEL: define target("wasm.externref.nonnull") @_Z9templatesu20non_null_externref_t(
// CHECK:         [[CALL:%.*]] = call target("wasm.externref.nonnull") @_Z8identityIu20non_null_externref_tET_S0_(target("wasm.externref.nonnull") {{%.*}})
// CHECK-NEXT:    ret target("wasm.externref.nonnull") [[CALL]]
__non_null_externref_t templates(__non_null_externref_t non_null) {
  return identity(non_null);
}
