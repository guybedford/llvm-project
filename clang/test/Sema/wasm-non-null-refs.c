// RUN: %clang_cc1 -fsyntax-only -verify=expected,conly -triple wasm32 -Wno-unused-value -target-feature +reference-types -target-feature +gc %s
// RUN: %clang_cc1 -x c++ -std=c++17 -fsyntax-only -verify=expected,cpp -triple wasm32 -Wno-unused-value -target-feature +reference-types -target-feature +gc %s
// RUN: %clang_cc1 -fsyntax-only -verify=nogc -triple wasm32 -target-feature +reference-types -DNO_GC %s
// RUN: %clang_cc1 -x c++ -std=c++17 -fsyntax-only -verify=nogc -triple wasm32 -target-feature +reference-types -DNO_GC %s

#ifdef NO_GC

// The non-nullable type and the builtin producing it need the gc feature.
__externref_t still_declared;
__non_null_externref_t not_declared; // nogc-error {{unknown type name '__non_null_externref_t'}}

void no_gc(__externref_t ref) {
  __builtin_wasm_ref_as_non_null_extern(ref); // nogc-error {{'__builtin_wasm_ref_as_non_null_extern' needs target feature gc}}
}

#else

// A non-nullable reference has no default value, so unlike __externref_t it
// cannot have static or thread storage duration.
__non_null_externref_t r1;        // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}
extern __non_null_externref_t r2; // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}
static __non_null_externref_t r3; // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}
#ifdef __cplusplus
thread_local __non_null_externref_t r4;         // cpp-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have thread storage duration}}
#else
_Thread_local __non_null_externref_t r4;        // conly-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have thread storage duration}}
#endif
typedef __non_null_externref_t non_null_t;
typedef const __non_null_externref_t const_non_null_t;
non_null_t r5;       // expected-error {{variable of non-nullable WebAssembly reference type 'non_null_t' (aka '__non_null_externref_t') cannot have static storage duration}}
const_non_null_t r6; // expected-error {{variable of non-nullable WebAssembly reference type 'const_non_null_t' (aka 'const __non_null_externref_t') cannot have static storage duration}}

__externref_t nullable_global;

__non_null_externref_t *t1;      // expected-error {{pointer to WebAssembly reference type is not allowed}}
__non_null_externref_t **t2;     // expected-error {{pointer to WebAssembly reference type is not allowed}}
__non_null_externref_t ******t3; // expected-error {{pointer to WebAssembly reference type is not allowed}}

// Nor can it be the element type of a table.
static __non_null_externref_t t4[3];      // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
static __non_null_externref_t t5[];       // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
__non_null_externref_t t7[0];             // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
static __non_null_externref_t t8[0][0];   // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
static __non_null_externref_t (*t9)[0];   // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
static __non_null_externref_t table[0];   // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
static non_null_t typedef_table[0];       // expected-error {{WebAssembly table cannot have non-nullable element type 'non_null_t' (aka '__non_null_externref_t')}}
static __non_null_externref_t other_table[0] = {}; // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}

static __externref_t nullable_table[0];
typedef void (*__funcref funcref_t)();
static funcref_t funcref_table[0];

struct s {
  __non_null_externref_t f1;       // expected-error {{field has sizeless type '__non_null_externref_t'}}
  __non_null_externref_t f2[0];    // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
  __non_null_externref_t f3[];     // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
  __non_null_externref_t *f5;      // expected-error {{pointer to WebAssembly reference type is not allowed}}
  __non_null_externref_t ****f6;   // expected-error {{pointer to WebAssembly reference type is not allowed}}
  __non_null_externref_t (*f7)[0]; // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
};

union u {
  __non_null_externref_t f1;       // expected-error {{field has sizeless type '__non_null_externref_t'}}
  __non_null_externref_t f2[0];    // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
  __non_null_externref_t *f5;      // expected-error {{pointer to WebAssembly reference type is not allowed}}
};

void illegal_argument_1(__non_null_externref_t table[]);     // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
void illegal_argument_3(__non_null_externref_t *table);      // expected-error {{pointer to WebAssembly reference type is not allowed}}
void illegal_argument_4(__non_null_externref_t ***table);    // expected-error {{pointer to WebAssembly reference type is not allowed}}
void illegal_argument_5(__non_null_externref_t (*table)[0]); // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
void illegal_argument_6(__non_null_externref_t table[0]);    // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}

__non_null_externref_t *illegal_return_1();      // expected-error {{pointer to WebAssembly reference type is not allowed}}
__non_null_externref_t ***illegal_return_2();    // expected-error {{pointer to WebAssembly reference type is not allowed}}
__non_null_externref_t (*illegal_return_3())[0]; // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}

void varargs(int, ...);

// Parameters, return values and locals are allowed.
__non_null_externref_t func(__non_null_externref_t ref) {
  &ref;                                 // expected-error {{cannot take address of WebAssembly reference}}
  int foo = 40;
  (__non_null_externref_t *)(&foo);     // expected-error {{pointer to WebAssembly reference type is not allowed}}
  (__non_null_externref_t ****)(&foo);  // expected-error {{pointer to WebAssembly reference type is not allowed}}
  sizeof(ref);                          // expected-error {{invalid application of 'sizeof' to sizeless type '__non_null_externref_t'}}
  sizeof(__non_null_externref_t);       // expected-error {{invalid application of 'sizeof' to sizeless type '__non_null_externref_t'}}
  sizeof(__non_null_externref_t[0]);    // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
  sizeof(__non_null_externref_t *);     // expected-error {{pointer to WebAssembly reference type is not allowed}}
  sizeof(__non_null_externref_t ***);   // expected-error {{pointer to WebAssembly reference type is not allowed}};
  // expected-warning@+1 {{'_Alignof' applied to an expression is a GNU extension}}
  _Alignof(ref);                        // expected-error {{invalid application of 'alignof' to sizeless type '__non_null_externref_t'}}
  _Alignof(__non_null_externref_t);     // expected-error {{invalid application of 'alignof' to sizeless type '__non_null_externref_t'}}
  _Alignof(__non_null_externref_t[]);   // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
  _Alignof(__non_null_externref_t *);   // expected-error {{pointer to WebAssembly reference type is not allowed}}
  varargs(1, ref);                      // expected-error {{cannot pass expression of type '__non_null_externref_t' to variadic function}}

  __non_null_externref_t lt1[0];        // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
  static __non_null_externref_t lt2[0]; // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}

  __non_null_externref_t uninitialized;
  __non_null_externref_t local = ref;
  const __non_null_externref_t const_local = ref;
  static __non_null_externref_t static_local;       // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}
  static __non_null_externref_t static_init = ref;  // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}
  extern __non_null_externref_t extern_local;       // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}

  local = const_local;
  return uninitialized;
}

// The two externref types are distinct, so these are different function types.
void redeclared(__externref_t);          // conly-note {{previous declaration is here}}
void redeclared(__non_null_externref_t); // conly-error {{conflicting types for 'redeclared'}}

__externref_t returns_nullable(void);
__non_null_externref_t returns_non_null(void);
void takes_nullable(__externref_t);
void takes_non_null(__non_null_externref_t); // conly-note 3 {{passing argument to parameter here}} \
                                                cpp-note 3 {{candidate function not viable: no known conversion from '__externref_t' to '__non_null_externref_t' for 1st argument}}

// Non-nullable to nullable is an implicit conversion.
__externref_t widen(__non_null_externref_t non_null, int cond) {
  __externref_t init = non_null;
  const __externref_t const_init = non_null;
  __externref_t init_call = returns_non_null();
  __externref_t assigned;
  assigned = non_null;
  assigned = returns_non_null();
  takes_nullable(non_null);
  takes_nullable(returns_non_null());
  takes_nullable((__externref_t)non_null);

  __externref_t nullable = returns_nullable();
  takes_nullable(cond ? non_null : nullable);
  takes_nullable(cond ? nullable : non_null);
  takes_nullable(cond ? returns_nullable() : returns_non_null());
  takes_non_null(cond ? non_null : returns_non_null());
  // The result of a mixed conditional is nullable.
  takes_non_null(cond ? non_null : nullable); // conly-error {{passing '__externref_t' to parameter of incompatible type '__non_null_externref_t'}} \
                                                 cpp-error {{no matching function for call to 'takes_non_null'}}
  takes_non_null(cond ? nullable : non_null); // conly-error {{passing '__externref_t' to parameter of incompatible type '__non_null_externref_t'}} \
                                                 cpp-error {{no matching function for call to 'takes_non_null'}}
  if (cond)
    return returns_non_null();
  return non_null;
}

// Nullable to non-nullable is never implicit and cannot be spelled as a cast.
__non_null_externref_t narrow(__externref_t nullable, __non_null_externref_t non_null) {
  __non_null_externref_t init = nullable; // conly-error {{initializing '__non_null_externref_t' with an expression of incompatible type '__externref_t'}} \
                                             cpp-error {{cannot initialize a variable of type '__non_null_externref_t' with an lvalue of type '__externref_t'}}
  __non_null_externref_t init_call = returns_nullable(); // conly-error {{initializing '__non_null_externref_t' with an expression of incompatible type '__externref_t'}} \
                                                            cpp-error {{cannot initialize a variable of type '__non_null_externref_t' with an rvalue of type '__externref_t'}}
  non_null = nullable;      // conly-error {{assigning to '__non_null_externref_t' from incompatible type '__externref_t'}} \
                               cpp-error {{assigning to '__non_null_externref_t' from incompatible type '__externref_t'}}
  takes_non_null(nullable); // conly-error {{passing '__externref_t' to parameter of incompatible type '__non_null_externref_t'}} \
                               cpp-error {{no matching function for call to 'takes_non_null'}}
  (__non_null_externref_t)nullable; // conly-error {{used type '__non_null_externref_t' where arithmetic or pointer type is required}} \
                                       cpp-error {{C-style cast from '__externref_t' to '__non_null_externref_t' is not allowed}}
  non_null = __builtin_wasm_ref_null_extern(); // conly-error {{assigning to '__non_null_externref_t' from incompatible type '__externref_t'}} \
                                                  cpp-error {{assigning to '__non_null_externref_t' from incompatible type '__externref_t'}}
  non_null = __builtin_wasm_table_get(nullable_table, 0); // conly-error {{assigning to '__non_null_externref_t' from incompatible type '__externref_t'}} \
                                                             cpp-error {{assigning to '__non_null_externref_t' from incompatible type '__externref_t'}}
  return nullable; // conly-error {{returning '__externref_t' from a function with incompatible result type '__non_null_externref_t'}} \
                      cpp-error {{cannot initialize return object of type '__non_null_externref_t' with an lvalue of type '__externref_t'}}
}

// There are no conversions to or from anything else.
void unrelated(__non_null_externref_t non_null, funcref_t funcref, void *ptr, int i) {
  non_null = 0;       // conly-error {{assigning to '__non_null_externref_t' from incompatible type 'int'}} \
                         cpp-error {{assigning to '__non_null_externref_t' from incompatible type 'int'}}
  non_null = ptr;     // conly-error {{assigning to '__non_null_externref_t' from incompatible type 'void *'}} \
                         cpp-error {{assigning to '__non_null_externref_t' from incompatible type 'void *'}}
  non_null = funcref; // conly-error {{assigning to '__non_null_externref_t' from incompatible type 'funcref_t'}} \
                         cpp-error {{assigning to '__non_null_externref_t' from incompatible type 'funcref_t'}}
  i = non_null;       // conly-error {{assigning to 'int' from incompatible type '__non_null_externref_t'}} \
                         cpp-error {{assigning to 'int' from incompatible type '__non_null_externref_t'}}
  ptr = non_null;     // conly-error {{assigning to 'void *' from incompatible type '__non_null_externref_t'}} \
                         cpp-error {{assigning to 'void *' from incompatible type '__non_null_externref_t'}}
  funcref = non_null; // conly-error {{assigning to 'funcref_t' (aka 'void (__funcref *)()') from incompatible type '__non_null_externref_t'}} \
                         cpp-error {{assigning to 'funcref_t' (aka 'void (__funcref *)()') from incompatible type '__non_null_externref_t'}}
  (__non_null_externref_t)i;   // conly-error {{used type '__non_null_externref_t' where arithmetic or pointer type is required}} \
                                  cpp-error {{C-style cast from 'int' to '__non_null_externref_t' is not allowed}}
  (__non_null_externref_t)ptr; // conly-error {{used type '__non_null_externref_t' where arithmetic or pointer type is required}} \
                                  cpp-error {{C-style cast from 'void *' to '__non_null_externref_t' is not allowed}}
  (int)non_null;               // conly-error {{operand of type '__non_null_externref_t' where arithmetic or pointer type is required}} \
                                  cpp-error {{C-style cast from '__non_null_externref_t' to 'int' is not allowed}}
  (void *)non_null;            // conly-error {{operand of type '__non_null_externref_t' where arithmetic or pointer type is required}} \
                                  cpp-error {{cannot cast from type '__non_null_externref_t' to pointer type 'void *'}}
  i ? non_null : 0;            // expected-error {{incompatible operand types ('__non_null_externref_t' and 'int')}}
  i ? funcref : non_null;      // expected-error {{incompatible operand types ('funcref_t' (aka 'void (__funcref *)()') and '__non_null_externref_t')}}
  (void)non_null;
}

// A non-nullable reference cannot be value-initialized.
void no_default_value(void) {
  __non_null_externref_t empty = {};  // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}
  takes_non_null((__non_null_externref_t){}); // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}
  __externref_t nullable_empty = {};
}

#define EXPR_HAS_TYPE(expr, type) _Generic((expr), type : 1, default : 0)

void test_ref_as_non_null(__externref_t nullable, __non_null_externref_t non_null, int i) {
  __non_null_externref_t narrowed = __builtin_wasm_ref_as_non_null_extern(nullable);
  narrowed = __builtin_wasm_ref_as_non_null_extern(non_null);
  narrowed = __builtin_wasm_ref_as_non_null_extern(returns_nullable());
  __builtin_wasm_ref_as_non_null_extern(nullable);
  __builtin_wasm_ref_as_non_null_extern();                   // expected-error {{too few arguments to function call, expected 1, have 0}}
  __builtin_wasm_ref_as_non_null_extern(nullable, nullable); // expected-error {{too many arguments to function call, expected 1, have 2}}
  __builtin_wasm_ref_as_non_null_extern(i);                  // expected-error {{1st argument must be an externref}}
  __builtin_wasm_ref_as_non_null_extern(0);                  // expected-error {{1st argument must be an externref}}
  __builtin_wasm_ref_as_non_null_extern(nullable_table);     // expected-error {{1st argument must be an externref}}
  __builtin_wasm_ref_as_non_null_extern(funcref_table);      // expected-error {{1st argument must be an externref}}

#ifndef __cplusplus
  _Static_assert(EXPR_HAS_TYPE(__builtin_wasm_ref_as_non_null_extern(nullable), __non_null_externref_t), "");
  _Static_assert(!EXPR_HAS_TYPE(__builtin_wasm_ref_as_non_null_extern(nullable), __externref_t), "");
  _Static_assert(EXPR_HAS_TYPE(i ? nullable : non_null, __externref_t), "");
  _Static_assert(EXPR_HAS_TYPE(i ? non_null : nullable, __externref_t), "");
  _Static_assert(EXPR_HAS_TYPE(i ? non_null : non_null, __non_null_externref_t), "");
  // Builtins returning an externref keep returning the nullable type.
  _Static_assert(EXPR_HAS_TYPE(__builtin_wasm_ref_null_extern(), __externref_t), "");
  _Static_assert(EXPR_HAS_TYPE(__builtin_wasm_table_get(nullable_table, 0), __externref_t), "");
#endif
}

#ifndef __cplusplus
// The conversion also takes part in C overload resolution.
__attribute__((overloadable)) int overloaded(__externref_t);
__attribute__((overloadable)) char overloaded(__non_null_externref_t);
__attribute__((overloadable)) int overloaded_nullable(__externref_t);
__attribute__((overloadable)) char overloaded_nullable(int);
__attribute__((overloadable)) int overloaded_non_null(__non_null_externref_t); // conly-note {{candidate function not viable: no known conversion from '__externref_t' to '__non_null_externref_t' for 1st argument}}
__attribute__((overloadable)) char overloaded_non_null(int);                   // conly-note {{candidate function not viable: no known conversion from '__externref_t' to 'int' for 1st argument}}

void test_overloadable(__externref_t nullable, __non_null_externref_t non_null) {
  _Static_assert(EXPR_HAS_TYPE(overloaded(nullable), int), "");
  _Static_assert(EXPR_HAS_TYPE(overloaded(non_null), char), "");
  _Static_assert(EXPR_HAS_TYPE(overloaded_nullable(non_null), int), "");
  overloaded_non_null(nullable); // conly-error {{no matching function for call to 'overloaded_non_null'}}
}
#endif

// Builtins taking an externref value accept a non-nullable one.
void test_value_builtins(__non_null_externref_t non_null, funcref_t funcref, int i) {
  __builtin_wasm_ref_is_null_extern(non_null);
  __builtin_wasm_ref_is_null_extern(returns_non_null());
  __builtin_wasm_table_set(nullable_table, i, non_null);
  __builtin_wasm_table_grow(nullable_table, non_null, i);
  __builtin_wasm_table_fill(nullable_table, i, non_null, i);
  __builtin_wasm_table_set(funcref_table, i, non_null);     // expected-error {{3rd argument must match the element type of the WebAssembly table in the 1st argument}}
  __builtin_wasm_table_grow(funcref_table, non_null, i);    // expected-error {{2nd argument must match the element type of the WebAssembly table in the 1st argument}}
  __builtin_wasm_table_fill(funcref_table, i, non_null, i); // expected-error {{3rd argument must match the element type of the WebAssembly table in the 1st argument}}
  __builtin_wasm_table_size(non_null);                      // expected-error {{1st argument must be a WebAssembly table}}
}

// A reference cannot be reinterpreted as another kind of reference, or made
// up.
void no_reinterpretation(__externref_t nullable, __non_null_externref_t non_null) {
  __builtin_bit_cast(__non_null_externref_t, nullable); // expected-error {{'__builtin_bit_cast' from '__externref_t' to '__non_null_externref_t' is not allowed; WebAssembly reference types cannot be reinterpreted}}
  __builtin_bit_cast(__externref_t, non_null);          // expected-error {{'__builtin_bit_cast' from '__non_null_externref_t' to '__externref_t' is not allowed; WebAssembly reference types cannot be reinterpreted}}
  __builtin_bit_cast(__non_null_externref_t, non_null);
  __builtin_nondeterministic_value(non_null);           // expected-error {{1st argument must be a vector, integer or floating-point type (was '__non_null_externref_t')}}
}

#endif
