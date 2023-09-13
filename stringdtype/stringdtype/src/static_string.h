#ifndef _NPY_STATIC_STRING_H
#define _NPY_STATIC_STRING_H

#include "stdlib.h"
#include "string.h"

typedef struct npy_static_string {
    size_t size;
    char *buf;
} npy_static_string;

// represents the empty string and can be passed safely to npy_static_string
// API functions
extern const npy_static_string EMPTY_STRING;
// represents a sentinel value, *CANNOT* be passed safely to npy_static_string
// API functions, use npy_string_isnull to check if a value is null before
// working with it.
extern const npy_static_string NULL_STRING;

// Allocates a new buffer for *to_init*, filling with the copied contents of
// the first *size*entries in *init*, which must be valid and initialized
// beforehand, and sets *to_init->size* to *size*. Returns -1 if malloc fails
// and -2 if *to_init* is not NULL. Returns 0 on success.
int
npy_string_newsize(const char *init, size_t size, npy_static_string *to_init);

// Sets len to 0 and if str->buf is not already NULL, frees it and sets it to
// NULL. Cannot fail.
void
npy_string_free(npy_static_string *str);

// Copies the contents of *in* into *out*. Allocates a new string buffer for
// *out* and assumes that *out* is uninitialized. Returns -1 if malloc fails
// and -2 if *out* is not initialized. npy_string_free *must* be called before
// this is called if *in* points to an existing string. Returns 0 on success.
int
npy_string_dup(const npy_static_string *in, npy_static_string *out);

// Allocates a new string buffer for *out* with enough capacity to store
// *size* bytes of text. Does not do any initialization, the caller must
// initialize the string buffer. Returns -1 if malloc fails and -2 if *out* is
// not NULL. Returns 0 on success.
int
npy_string_newemptysize(size_t size, npy_static_string *out);

// Determine if *in* corresponds to a NULL npy_static_string struct (e.g. len
// is zero and buf is NULL. Returns 1 if this is the case and zero otherwise.
// Cannot fail.
int
npy_string_isnull(const npy_static_string *in);

// Compare two strings. Has the same sematics as if strcmp were passed
// null-terminated C strings with the content of *s1* and *s2*.
int
npy_string_cmp(const npy_static_string *s1, const npy_static_string *s2);

#endif /*_NPY_STATIC_STRING_H */
