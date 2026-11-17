# RUN: rm -rf %t && split-file %s %t
# RUN: llvm-mc -filetype=obj -triple=wasm32-unknown-unknown -mattr=+reference-types,+gc %t/main.s -o %t/main.o
# RUN: llvm-mc -filetype=obj -triple=wasm32-unknown-unknown -mattr=+reference-types,+gc %t/def.s -o %t/def.o
# RUN: llvm-mc -filetype=obj -triple=wasm32-unknown-unknown -mattr=+reference-types,+gc %t/mismatch.s -o %t/mismatch.o

## A function taking and returning non-nullable references is defined in one
## object and called from another.
# RUN: wasm-ld --fatal-warnings -o %t/out.wasm %t/main.o %t/def.o
# RUN: obj2yaml %t/out.wasm | FileCheck %s

# RUN: wasm-ld --fatal-warnings -r -o %t/out.o %t/main.o %t/def.o
# RUN: obj2yaml %t/out.o | FileCheck %s

## (ref extern) and externref are different types.
# RUN: not wasm-ld --fatal-warnings -o %t/mismatch.wasm %t/mismatch.o %t/def.o 2>&1 | FileCheck %s --check-prefix=MISMATCH
# RUN: wasm-ld -o %t/mismatch.wasm %t/mismatch.o %t/def.o 2>&1 | FileCheck %s --check-prefix=MISMATCH

# MISMATCH:      function signature mismatch: non_null
# MISMATCH-NEXT: >>> defined as (externref) -> externref in {{.*}}mismatch.o
# MISMATCH-NEXT: >>> defined as ((ref extern)) -> (ref extern) in {{.*}}def.o

## The signature of the indirect call is the same type as that of non_null.
#      CHECK:  - Type:            TYPE
# CHECK-NEXT:    Signatures:
# CHECK-NEXT:      - Index:           0
# CHECK-NEXT:        ParamTypes:
# CHECK-NEXT:          - REF_EXTERN
# CHECK-NEXT:        ReturnTypes:
# CHECK-NEXT:          - REF_EXTERN
# CHECK-NEXT:      - Index:           1
# CHECK-NEXT:        ParamTypes:
# CHECK-NEXT:          - EXTERNREF
# CHECK-NEXT:          - EXTERNREF
# CHECK-NEXT:        ReturnTypes:
# CHECK-NEXT:          - REF_EXTERN
# CHECK-NEXT:      - Index:           2
# CHECK-NEXT:        ParamTypes:
# CHECK-NEXT:          - EXTERNREF
# CHECK-NEXT:        ReturnTypes:     []
# CHECK-NEXT:      - Index:           3
# CHECK-NEXT:        ParamTypes:
# CHECK-NEXT:          - EXTERNREF
# CHECK-NEXT:        ReturnTypes:
# CHECK-NEXT:          - REF_EXTERN
# CHECK-NEXT:  - Type:            IMPORT
#      CHECK:      - Module:          'wasm:js-string'
# CHECK-NEXT:        Field:           concat
# CHECK-NEXT:        Kind:            FUNCTION
# CHECK-NEXT:        SigIndex:        1

#--- main.s
.functype non_null ((ref extern)) -> ((ref extern))
.functype to_non_null (externref) -> ((ref extern))
.functype concat (externref, externref) -> ((ref extern))
.import_module concat, "wasm:js-string"
.import_name concat, concat

.globl _start
_start:
  .functype _start (externref) -> ()
  local.get 0
  call to_non_null
  call non_null
  local.get 0
  call concat
  i32.const 0
  call_indirect ((ref extern)) -> ((ref extern))
  drop
  end_function

#--- def.s
.globl non_null
non_null:
  .functype non_null ((ref extern)) -> ((ref extern))
  local.get 0
  end_function

.globl to_non_null
to_non_null:
  .functype to_non_null (externref) -> ((ref extern))
  local.get 0
  ref.as_non_null
  end_function

#--- mismatch.s
.functype non_null (externref) -> (externref)

.globl _start
_start:
  .functype _start (externref) -> ()
  local.get 0
  call non_null
  drop
  end_function
