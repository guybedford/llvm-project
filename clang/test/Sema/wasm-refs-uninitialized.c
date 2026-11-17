// RUN: %clang_cc1 -fsyntax-only -verify -triple wasm32 -Wuninitialized -target-feature +reference-types -target-feature +gc %s

void take_nullable(__externref_t);
void take_non_null(__non_null_externref_t);
__non_null_externref_t get_non_null(void);

void uninitialized(int cond) {
  __externref_t nullable; // expected-note {{variable 'nullable' is declared here}}
  take_nullable(nullable); // expected-warning {{variable 'nullable' is uninitialized when used here}}

  __non_null_externref_t non_null; // expected-note {{variable 'non_null' is declared here}}
  take_non_null(non_null); // expected-warning {{variable 'non_null' is uninitialized when used here}}

  __non_null_externref_t sometimes; // expected-note {{variable 'sometimes' is declared here}}
  if (cond) // expected-warning {{variable 'sometimes' is used uninitialized whenever 'if' condition is false}} \
               expected-note {{remove the 'if' if its condition is always true}}
    sometimes = get_non_null();
  take_non_null(sometimes); // expected-note {{uninitialized use occurs here}}

  __non_null_externref_t initialized = get_non_null();
  take_non_null(initialized);
}
