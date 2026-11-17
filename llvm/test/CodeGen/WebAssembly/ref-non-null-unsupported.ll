; RUN: not llc -mattr=+reference-types < %s 2>&1 | FileCheck %s --check-prefix=NO-GC
; RUN: not llc -mattr=+reference-types,+gc -exception-model=emscripten < %s 2>&1 | FileCheck %s --check-prefix=EM-EH

target triple = "wasm32-unknown-unknown"

%externref = type target("wasm.externref")
%nonnull = type target("wasm.externref.nonnull")

declare %nonnull @llvm.wasm.ref.as_non_null.extern(%externref)
declare void @take_non_null(%nonnull)
declare i32 @__gxx_personality_v0(...)

; NO-GC: error: {{.*}} in function narrow {{.*}}: WebAssembly non-nullable reference types require the 'gc' feature
; NO-GC: error: {{.*}} in function narrow {{.*}}: ref.as_non_null requires the 'gc' feature
define %nonnull @narrow(%externref %r) {
  %nn = call %nonnull @llvm.wasm.ref.as_non_null.extern(%externref %r)
  ret %nonnull %nn
}

; NO-GC: error: {{.*}} in function invoke {{.*}}: WebAssembly non-nullable reference types require the 'gc' feature
; EM-EH: LLVM ERROR: Emscripten EH/SjLj does not support non-nullable reference types: __invoke_void_target("wasm.externref.nonnull"): (i32, (ref extern)) -> ()
define void @invoke(%nonnull %r) personality ptr @__gxx_personality_v0 {
entry:
  invoke void @take_non_null(%nonnull %r)
          to label %done unwind label %lpad
lpad:
  %lp = landingpad { ptr, i32 }
          cleanup
  resume { ptr, i32 } %lp
done:
  ret void
}
