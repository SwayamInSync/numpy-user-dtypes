#ifndef _NPY_DTYPE_H
#define _NPY_DTYPE_H

// clang-format off
#include <Python.h>
#include "structmember.h"
// clang-format on

#include "static_string.h"

#define PY_ARRAY_UNIQUE_SYMBOL stringdtype_ARRAY_API
#define NPY_NO_DEPRECATED_API NPY_2_0_API_VERSION
#define NPY_TARGET_VERSION NPY_2_0_API_VERSION
#define NO_IMPORT_ARRAY
#include "numpy/arrayobject.h"
#include "numpy/experimental_dtype_api.h"
#include "numpy/halffloat.h"
#include "numpy/ndarraytypes.h"
#include "numpy/npy_math.h"
#include "numpy/ufuncobject.h"

#define NPY_STRING_ACQUIRE_ALLOCATOR(descr)                           \
    if (!PyThread_acquire_lock(descr->allocator_lock, NOWAIT_LOCK)) { \
        PyThread_acquire_lock(descr->allocator_lock, WAIT_LOCK);      \
    }

#define NPY_STRING_ACQUIRE_ALLOCATOR2(descr1, descr2) \
    NPY_STRING_ACQUIRE_ALLOCATOR(descr1)              \
    if (descr1 != descr2) {                           \
        NPY_STRING_ACQUIRE_ALLOCATOR(descr2)          \
    }

#define NPY_STRING_ACQUIRE_ALLOCATOR3(descr1, descr2, descr3) \
    NPY_STRING_ACQUIRE_ALLOCATOR(descr1)                      \
    if (descr1 != descr2) {                                   \
        NPY_STRING_ACQUIRE_ALLOCATOR(descr2)                  \
    }                                                         \
    if (descr1 != descr3 && descr2 != descr3) {               \
        NPY_STRING_ACQUIRE_ALLOCATOR(descr3)                  \
    }

#define NPY_STRING_RELEASE_ALLOCATOR(descr) \
    PyThread_release_lock(descr->allocator_lock);
#define NPY_STRING_RELEASE_ALLOCATOR2(descr1, descr2) \
    NPY_STRING_RELEASE_ALLOCATOR(descr1);             \
    if (descr1 != descr2) {                           \
        NPY_STRING_RELEASE_ALLOCATOR(descr2);         \
    }
#define NPY_STRING_RELEASE_ALLOCATOR3(descr1, descr2, descr3) \
    NPY_STRING_RELEASE_ALLOCATOR(descr1);                     \
    if (descr1 != descr2) {                                   \
        NPY_STRING_RELEASE_ALLOCATOR(descr2);                 \
    }                                                         \
    if (descr1 != descr3 && descr2 != descr3) {               \
        NPY_STRING_RELEASE_ALLOCATOR(descr3);                 \
    }

// not publicly exposed by the static string library so we need to define
// this here so we can define `elsize` and `alignment` on the descr
//
// if the layout of npy_packed_static_string ever changes in the future
// this may need to be updated.
#define SIZEOF_NPY_PACKED_STATIC_STRING 2 * sizeof(size_t)
#define ALIGNOF_NPY_PACKED_STATIC_STRING _Alignof(size_t)

typedef struct {
    PyArray_Descr base;
    PyObject *na_object;
    int coerce;
    int has_nan_na;
    int has_string_na;
    int array_owned;
    npy_static_string default_string;
    char packed_default_string[SIZEOF_NPY_PACKED_STATIC_STRING];
    npy_static_string na_name;
    char packed_na_name[SIZEOF_NPY_PACKED_STATIC_STRING];
    PyThread_type_lock *allocator_lock;
    // the allocator should only be directly accessed after
    // acquiring the allocator_lock and the lock should
    // be released immediately after the allocator is
    // no longer needed
    npy_string_allocator *allocator;
} StringDTypeObject;

typedef struct {
    PyArray_DTypeMeta base;
} StringDType_type;

extern StringDType_type StringDType;
extern PyTypeObject *StringScalar_Type;

PyObject *
new_stringdtype_instance(PyObject *na_object, int coerce);

int
init_string_dtype(void);

// Assumes that the caller has already acquired the allocator locks for both
// descriptors
int
_compare(void *a, void *b, StringDTypeObject *descr_a,
         StringDTypeObject *descr_b);

int
init_string_na_object(PyObject *mod);

int
stringdtype_setitem(StringDTypeObject *descr, PyObject *obj, char **dataptr);

// set the python error indicator when the gil is released
void
gil_error(PyObject *type, const char *msg);

// the locks on both allocators must be acquired before calling this function
int
free_and_copy(npy_string_allocator *in_allocator,
              npy_string_allocator *out_allocator,
              const npy_packed_static_string *in,
              npy_packed_static_string *out, const char *location);

PyArray_Descr *
stringdtype_finalize_descr(PyArray_Descr *dtype);

int
_eq_comparison(int scoerce, int ocoerce, PyObject *sna, PyObject *ona);

#endif /*_NPY_DTYPE_H*/
