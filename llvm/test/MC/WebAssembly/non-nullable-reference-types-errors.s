# RUN: not llvm-mc -triple=wasm32-unknown-unknown -mattr=+reference-types,+exception-handling,+gc %s 2>&1 | FileCheck %s

.functype take_non_null ((ref extern)) -> ()
.functype take_nullable (externref) -> ()

# A nullable reference is not accepted where a non-nullable one is expected.

nullable_arg_to_non_null_param:
  .functype nullable_arg_to_non_null_param (externref) -> ()
  local.get 0
# CHECK: :[[@LINE+1]]:3: error: type mismatch, expected [(ref extern)] but got [externref]
  call take_non_null
  end_function

nullable_to_non_null_result:
  .functype nullable_to_non_null_result (externref) -> ((ref extern))
  local.get 0
# CHECK: :[[@LINE+1]]:3: error: type mismatch, expected [(ref extern)] but got [externref]
  end_function

null_to_non_null_param:
  .functype null_to_non_null_param () -> ()
  ref.null_extern
# CHECK: :[[@LINE+1]]:3: error: type mismatch, expected [(ref extern)] but got [externref]
  call take_non_null
  end_function

# Subsumption only applies between references of the same heap type.

non_null_heap_type_mismatch:
  .functype non_null_heap_type_mismatch ((ref func)) -> ()
  local.get 0
# CHECK: :[[@LINE+1]]:3: error: type mismatch, expected [externref] but got [(ref func)]
  call take_nullable
  end_function

as_non_null_heap_type_mismatch:
  .functype as_non_null_heap_type_mismatch (funcref) -> ((ref extern))
  local.get 0
  ref.as_non_null
# CHECK: :[[@LINE+1]]:3: error: type mismatch, expected [(ref extern)] but got [(ref func)]
  end_function

as_non_null_non_ref:
  .functype as_non_null_non_ref () -> ()
  i32.const 0
# CHECK: :[[@LINE+1]]:3: error: type mismatch, expected [ref] but got [i32]
  ref.as_non_null
  drop
  end_function

as_non_null_empty_stack:
  .functype as_non_null_empty_stack () -> ()
# CHECK: :[[@LINE+1]]:3: error: type mismatch, expected [ref] but got []
  ref.as_non_null
  drop
  end_function

# Malformed reference types.

# CHECK: :[[@LINE+1]]:22: error: unknown heap type: i32
.functype bad0 ((ref i32)) -> ()

# CHECK: :[[@LINE+1]]:27: error: unknown heap type: any
.functype bad1 ((ref null any)) -> ()

# CHECK: :[[@LINE+1]]:18: error: Expected ref, instead got: extern
.functype bad2 ((extern)) -> ()

# CHECK: :[[@LINE+1]]:28: error: Expected ), instead got: ,
.functype bad3 ((ref extern, i32)) -> ()

# Non-nullable references are only supported in signatures.

non_null_local:
  .functype non_null_local () -> ()
# CHECK: :[[@LINE+1]]:10: error: non-nullable reference types are not supported in .local
  .local (ref extern)
  end_function
