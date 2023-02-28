#include <Python.h>

#define PY_ARRAY_UNIQUE_SYMBOL stringdtype_ARRAY_API
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/arrayobject.h"
#include "numpy/experimental_dtype_api.h"

#include "dtype.h"
#include "static_string.h"
#include "umath.h"

static PyObject *
_memory_usage(PyObject *NPY_UNUSED(self), PyObject *obj)
{
    if (!PyArray_Check(obj)) {
        PyErr_SetString(PyExc_TypeError,
                        "can only be called with ndarray object");
        return NULL;
    }

    PyArrayObject *arr = (PyArrayObject *)obj;

    PyArray_Descr *descr = PyArray_DESCR(arr);
    PyArray_DTypeMeta *dtype = NPY_DTYPE(descr);

    if (dtype != &StringDType) {
        PyErr_SetString(PyExc_TypeError,
                        "can only be called with a StringDType array");
        return NULL;
    }

    NpyIter *iter = NpyIter_New(
            arr, NPY_ITER_READONLY | NPY_ITER_EXTERNAL_LOOP | NPY_ITER_REFS_OK,
            NPY_KEEPORDER, NPY_NO_CASTING, NULL);

    if (iter == NULL) {
        return NULL;
    }

    NpyIter_IterNextFunc *iternext = NpyIter_GetIterNext(iter, NULL);

    if (iternext == NULL) {
        NpyIter_Deallocate(iter);
        return NULL;
    }

    char **dataptr = NpyIter_GetDataPtrArray(iter);
    npy_intp *strideptr = NpyIter_GetInnerStrideArray(iter);
    npy_intp *innersizeptr = NpyIter_GetInnerLoopSizePtr(iter);

    // initialize with the size of the internal buffer
    size_t memory_usage = PyArray_NBYTES(arr);
    size_t struct_size = sizeof(ss);

    do {
        ss **in = (ss **)*dataptr;
        npy_intp stride = *strideptr / descr->elsize;
        npy_intp count = *innersizeptr;

        while (count--) {
            // +1 byte for the null terminator
            memory_usage += (*in)->len + struct_size + 1;
            in += stride;
        }

    } while (iternext(iter));

    PyObject *ret = PyLong_FromSize_t(memory_usage);

    return ret;
}

static PyMethodDef string_methods[] = {
        {"_memory_usage", _memory_usage, METH_O,
         "get memory usage for an array"},
        {NULL},
};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        .m_name = "stringdtype_main",
        .m_size = -1,
        .m_methods = string_methods,
};

/* Module initialization function */
PyMODINIT_FUNC
PyInit__main(void)
{
    if (_import_array() < 0) {
        return NULL;
    }
    if (import_experimental_dtype_api(9) < 0) {
        return NULL;
    }

    PyObject *m = PyModule_Create(&moduledef);
    if (m == NULL) {
        return NULL;
    }

    PyObject *mod = PyImport_ImportModule("stringdtype");
    if (mod == NULL) {
        goto error;
    }
    StringScalar_Type =
            (PyTypeObject *)PyObject_GetAttrString(mod, "StringScalar");
    Py_DECREF(mod);

    if (StringScalar_Type == NULL) {
        goto error;
    }

    if (init_string_dtype() < 0) {
        goto error;
    }

    if (PyModule_AddObject(m, "StringDType", (PyObject *)&StringDType) < 0) {
        goto error;
    }

    if (init_ufuncs() < 0) {
        goto error;
    }

    return m;

error:
    Py_DECREF(m);
    return NULL;
}
