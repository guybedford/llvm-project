# RUN: llvm-mc -show-encoding -triple=wasm32-unknown-unknown -mattr=+reference-types,+exception-handling,+gc < %s | FileCheck %s
# RUN: llvm-mc -show-encoding -triple=wasm64-unknown-unknown -mattr=+reference-types,+exception-handling,+gc < %s | FileCheck %s

# RUN: llvm-mc -triple=wasm32-unknown-unknown -mattr=+reference-types,+exception-handling,+gc -filetype=obj -o %t.o %s
# RUN: obj2yaml %t.o | FileCheck --check-prefix=YAML %s
# RUN: llvm-objdump -s -j TYPE %t.o | FileCheck --check-prefix=BYTES %s

# The printed assembly assembles to the same object.
# RUN: llvm-mc -triple=wasm32-unknown-unknown -mattr=+reference-types,+exception-handling,+gc -o %t.s %s
# RUN: llvm-mc -triple=wasm32-unknown-unknown -mattr=+reference-types,+exception-handling,+gc -filetype=obj -o %t2.o %t.s
# RUN: cmp %t.o %t2.o

# RUN: not llvm-mc -triple=wasm32-unknown-unknown -mattr=+reference-types %s 2>&1 \
# RUN:   | FileCheck --check-prefix=NO-GC %s

# NO-GC: error: instruction requires: gc

# CHECK: .functype take_non_null ((ref extern)) -> ()
.functype take_non_null ((ref extern)) -> ()
# CHECK: .functype take_nullable (externref) -> ()
.functype take_nullable (externref) -> ()

extern_as_non_null:
# CHECK: .functype extern_as_non_null (externref) -> ((ref extern))
    .functype extern_as_non_null (externref) -> ((ref extern))
    local.get 0
# CHECK: ref.as_non_null # encoding: [0xd4]
    ref.as_non_null
    end_function

func_as_non_null:
# CHECK: .functype func_as_non_null (funcref) -> ((ref func))
    .functype func_as_non_null (funcref) -> ((ref func))
    local.get 0
# CHECK: ref.as_non_null # encoding: [0xd4]
    ref.as_non_null
    end_function

exn_as_non_null:
# CHECK: .functype exn_as_non_null (exnref) -> ((ref exn))
    .functype exn_as_non_null (exnref) -> ((ref exn))
    local.get 0
# CHECK: ref.as_non_null # encoding: [0xd4]
    ref.as_non_null
    end_function

# (ref null ht) is the same type as the htref shorthand.
nullable_longhand:
# CHECK: .functype nullable_longhand (externref, funcref, exnref) -> (externref)
    .functype nullable_longhand ((ref null extern), (ref null func), (ref null exn)) -> ((ref null extern))
    local.get 0
    end_function

concat:
# CHECK: .functype concat (externref, externref) -> ((ref extern))
    .functype concat (externref, externref) -> ((ref extern))
    local.get 0
    ref.as_non_null
    end_function

mixed:
# CHECK: .functype mixed (i32, (ref func), (ref exn), (ref extern), f64) -> ()
    .functype mixed (i32, (ref func), (ref exn), (ref extern), f64) -> ()
    end_function

# A non-nullable reference is accepted wherever the nullable reference of the
# same heap type is expected.
subsumption:
# CHECK: .functype subsumption ((ref extern)) -> (externref)
    .functype subsumption ((ref extern)) -> (externref)
    .local externref
    local.get 0
    call take_nullable
    local.get 0
    call take_non_null
    local.get 0
    local.set 1
    local.get 0
    ref.is_null
    drop
    local.get 0
    end_function

# ref.as_non_null also accepts a reference that is already non-nullable.
already_non_null:
# CHECK: .functype already_non_null ((ref extern)) -> ((ref extern))
    .functype already_non_null ((ref extern)) -> ((ref extern))
    local.get 0
    ref.as_non_null
    end_function

non_null_block_type:
# CHECK: .functype non_null_block_type (externref) -> ((ref extern))
    .functype non_null_block_type (externref) -> ((ref extern))
    local.get 0
# CHECK: block (externref) -> ((ref extern))
    block (externref) -> ((ref extern))
    ref.as_non_null
    end_block
    end_function

# ref.func produces a non-nullable reference.
ref_func:
# CHECK: .functype ref_func () -> ((ref func))
    .functype ref_func () -> ((ref func))
    ref.func ref_func
    end_function

# CHECK: .tagtype non_null_tag (ref extern)
.tagtype non_null_tag (ref extern)

# The exception reference of catch_ref is not null, and the payload of a tag
# is accepted by a label of its nullable type.
exceptions:
# CHECK: .functype exceptions ((ref extern)) -> ((ref exn))
    .functype exceptions ((ref extern)) -> ((ref exn))
    .local exnref
# CHECK: block (externref) -> (externref, (ref exn))
    local.get 0
    block (externref) -> (externref, (ref exn))
# CHECK: try_table (externref) -> (externref) (catch_ref non_null_tag 0)
    try_table (externref) -> (externref) (catch_ref non_null_tag 0)
    drop
    local.get 0
# CHECK: throw non_null_tag
    throw non_null_tag
    end_try_table
    unreachable
    end_block
    local.set 1
    drop
    local.get 1
    ref.as_non_null
    end_function

indirect:
# CHECK: .functype indirect ((ref extern), i32) -> ((ref extern))
    .functype indirect ((ref extern), i32) -> ((ref extern))
    local.get 0
    local.get 1
# CHECK: call_indirect __indirect_function_table, ((ref extern)) -> ((ref extern))
    call_indirect ((ref extern)) -> ((ref extern))
    end_function

# A local can be declared with the longhand of a nullable reference type.
nullable_longhand_local:
    .functype nullable_longhand_local () -> ()
# CHECK: .local externref, funcref
    .local (ref null extern), (ref null func)
    end_function

# An i31 reference converted to extern is a reference that is not null.
i31_as_extern:
# CHECK: .functype i31_as_extern () -> ((ref extern))
    .functype i31_as_extern () -> ((ref extern))
    i32.const 0
# CHECK: ref.i31 # encoding: [0xfb,0x1c]
    ref.i31
# CHECK: extern.convert_any # encoding: [0xfb,0x1b]
    extern.convert_any
    ref.as_non_null
    end_function

## Non-nullable references are encoded as 0x64 followed by the heap type.
# BYTES:      Contents of section TYPE:
# BYTES-NEXT:  0000 1160016f 01646f60 01700164 70600169
# BYTES-NEXT:  0010 01646960 036f7069 016f6002 6f6f0164
# BYTES-NEXT:  0020 6f60057f 64706469 646f7c00 6001646f
# BYTES-NEXT:  0030 016f6001 6f006001 646f0060 01646f01
# BYTES-NEXT:  0040 646f6000 01647060 01646f01 64696001
# BYTES-NEXT:  0050 6f026f64 6960016f 016f6002 646f7f01
# BYTES-NEXT:  0060 646f6000 00600001 646f

# YAML:      - Type:            TYPE
# YAML-NEXT:   Signatures:
# YAML-NEXT:     - Index:           0
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:     - Index:           1
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - FUNCREF
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_FUNC
# YAML-NEXT:     - Index:           2
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - EXNREF
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_EXN
# YAML-NEXT:     - Index:           3
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:         - FUNCREF
# YAML-NEXT:         - EXNREF
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:     - Index:           4
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:     - Index:           5
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - I32
# YAML-NEXT:         - REF_FUNC
# YAML-NEXT:         - REF_EXN
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:         - F64
# YAML-NEXT:       ReturnTypes:     []
# YAML-NEXT:     - Index:           6
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:     - Index:           7
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:       ReturnTypes:     []
# YAML-NEXT:     - Index:           8
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:       ReturnTypes:     []
# YAML-NEXT:     - Index:           9
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:     - Index:           10
# YAML-NEXT:       ParamTypes:      []
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_FUNC
# YAML-NEXT:     - Index:           11
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_EXN
# YAML-NEXT:     - Index:           12
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:         - REF_EXN
# YAML-NEXT:     - Index:           13
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - EXTERNREF
# YAML-NEXT:     - Index:           14
# YAML-NEXT:       ParamTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:         - I32
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_EXTERN
# YAML-NEXT:     - Index:           15
# YAML-NEXT:       ParamTypes:      []
# YAML-NEXT:       ReturnTypes:     []
# YAML-NEXT:     - Index:           16
# YAML-NEXT:       ParamTypes:      []
# YAML-NEXT:       ReturnTypes:
# YAML-NEXT:         - REF_EXTERN
