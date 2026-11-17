// RUN: %clang_cc1 -std=c++14 -fcxx-exceptions -fexceptions -fsyntax-only -verify -triple wasm32 -Wno-unused-value -target-feature +reference-types -target-feature +gc %s
// RUN: %clang_cc1 -std=c++20 -fcxx-exceptions -fexceptions -fsyntax-only -verify -triple wasm32 -Wno-unused-value -target-feature +reference-types -target-feature +gc %s

__externref_t returns_nullable();
__non_null_externref_t returns_non_null();

__non_null_externref_t &ref1 = returns_non_null();        // expected-error {{reference to WebAssembly reference type is not allowed}}
void illegal_argument(const __non_null_externref_t &ref); // expected-error {{reference to WebAssembly reference type is not allowed}}

struct S {
  static __non_null_externref_t member; // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}
};

template <typename T> T variable_template; // expected-error {{variable of non-nullable WebAssembly reference type '__non_null_externref_t' cannot have static storage duration}}
template <typename T> struct Table {
  static T table[0]; // expected-error {{WebAssembly table cannot have non-nullable element type '__non_null_externref_t'}}
};
template <typename T> T value_init() {
  return T(); // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}
}

void instantiate() {
  (void)variable_template<__externref_t>;
  (void)variable_template<__non_null_externref_t>; // expected-note {{in instantiation of variable template specialization 'variable_template' requested here}}
  Table<__non_null_externref_t> t;                 // expected-note {{in instantiation of template class 'Table<__non_null_externref_t>' requested here}}
  value_init<__externref_t>();
  value_init<__non_null_externref_t>();            // expected-note {{in instantiation of function template specialization 'value_init<__non_null_externref_t>' requested here}}
}

#if __cplusplus < 201703L
void exception_spec() throw(__non_null_externref_t); // expected-error {{WebAssembly reference type not allowed in exception specification}}
#endif

void exceptions(__non_null_externref_t non_null) {
  throw non_null;                    // expected-error {{cannot throw a WebAssembly reference type}}
  try {
  } catch (__non_null_externref_t T) { // expected-error {{cannot catch a WebAssembly reference type}}
  }
}

void capture(__non_null_externref_t non_null) {
  auto by_copy = [=]() { (void)non_null; }; // expected-error {{cannot capture WebAssembly reference}}
  auto by_ref = [&]() { (void)non_null; };  // expected-error {{cannot capture WebAssembly reference}}
}

void no_default_value() {
  __non_null_externref_t a{};                // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}
  __non_null_externref_t b = {};             // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}
  auto c = __non_null_externref_t();         // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}
  auto d = __non_null_externref_t{};         // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}
  __non_null_externref_t e{returns_non_null()};
  __non_null_externref_t f = {returns_non_null()};
  __non_null_externref_t g(returns_non_null());
  __non_null_externref_t uninitialized;
  __externref_t h{};
  auto i = __externref_t();
}
void default_argument(__non_null_externref_t = __non_null_externref_t()); // expected-error {{non-nullable WebAssembly reference type '__non_null_externref_t' has no default value}}

void casts(__externref_t nullable, __non_null_externref_t non_null) {
  // Non-nullable to nullable is an implicit conversion, so every cast
  // notation that can perform one accepts it.
  (void)static_cast<__externref_t>(non_null);
  (void)(__externref_t)non_null;
  (void)__externref_t(non_null);
  (void)__externref_t{non_null};
  (void)reinterpret_cast<__externref_t>(non_null); // expected-error {{reinterpret_cast from '__non_null_externref_t' to '__externref_t' is not allowed}}

  // Nullable to non-nullable needs __builtin_wasm_ref_as_non_null_extern.
  (void)static_cast<__non_null_externref_t>(nullable);      // expected-error {{static_cast from '__externref_t' to '__non_null_externref_t' is not allowed}}
  (void)reinterpret_cast<__non_null_externref_t>(nullable); // expected-error {{reinterpret_cast from '__externref_t' to '__non_null_externref_t' is not allowed}}
  (void)const_cast<__non_null_externref_t>(nullable);       // expected-error {{const_cast to '__non_null_externref_t', which is not a reference, pointer-to-object, or pointer-to-data-member}}
  (void)(__non_null_externref_t)nullable;                   // expected-error {{C-style cast from '__externref_t' to '__non_null_externref_t' is not allowed}}
  (void)__non_null_externref_t(nullable);                   // expected-error {{functional-style cast from '__externref_t' to '__non_null_externref_t' is not allowed}}
  (void)__non_null_externref_t{nullable};                   // expected-error {{cannot initialize a value of type '__non_null_externref_t' with an lvalue of type '__externref_t'}}
  (void)static_cast<__non_null_externref_t>(__builtin_wasm_ref_as_non_null_extern(nullable));
}

template <typename T, typename U> struct is_same { static const bool value = false; };
template <typename T> struct is_same<T, T> { static const bool value = true; };

static_assert(!is_same<__externref_t, __non_null_externref_t>::value, "");
static_assert(!is_same<void(__externref_t), void(__non_null_externref_t)>::value, "");
static_assert(!is_same<__externref_t(), __non_null_externref_t()>::value, "");
static_assert(is_same<decltype(__builtin_wasm_ref_as_non_null_extern(returns_nullable())), __non_null_externref_t>::value, "");
static_assert(is_same<decltype(true ? returns_nullable() : returns_non_null()), __externref_t>::value, "");
static_assert(is_same<decltype(true ? returns_non_null() : returns_nullable()), __externref_t>::value, "");
static_assert(is_same<decltype(true ? returns_non_null() : returns_non_null()), __non_null_externref_t>::value, "");

struct nullable_tag {};
struct non_null_tag {};

// With both overloads a non-nullable argument is an exact match.
nullable_tag both(__externref_t);
non_null_tag both(__non_null_externref_t);
static_assert(is_same<decltype(both(returns_nullable())), nullable_tag>::value, "");
static_assert(is_same<decltype(both(returns_non_null())), non_null_tag>::value, "");

// With only the nullable overload it converts.
nullable_tag only_nullable(__externref_t);
static_assert(is_same<decltype(only_nullable(returns_non_null())), nullable_tag>::value, "");

// The conversion ranks below an exact match but is otherwise an ordinary
// standard conversion.
nullable_tag ranked(__externref_t);
non_null_tag ranked(...);
static_assert(is_same<decltype(ranked(returns_non_null())), nullable_tag>::value, "");

non_null_tag only_non_null(__non_null_externref_t); // expected-note {{candidate function not viable: no known conversion from '__externref_t' to '__non_null_externref_t' for 1st argument}}
void no_narrowing(__externref_t nullable) {
  only_non_null(nullable); // expected-error {{no matching function for call to 'only_non_null'}}
}

// An empty initializer list does not convert to a non-nullable reference.
non_null_tag from_empty_list(__non_null_externref_t);
nullable_tag from_empty_list(int);
static_assert(is_same<decltype(from_empty_list({})), nullable_tag>::value, "");

template <typename T> T deduce(T);
static_assert(is_same<decltype(deduce(returns_non_null())), __non_null_externref_t>::value, "");

struct Convertible {
  operator __non_null_externref_t();
};
void user_defined(Convertible c) {
  // A user-defined conversion may be followed by the widening conversion.
  __externref_t nullable = c;
  __non_null_externref_t non_null = c;
  only_nullable(c);
}
