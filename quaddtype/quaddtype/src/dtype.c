#include "dtype.h"
#include "abstract.h"
#include "casts.h"

PyTypeObject *QuadScalar_Type = NULL;

QuadDTypeObject *
new_quaddtype_instance(void)
{
    QuadDTypeObject *new =
            (QuadDTypeObject *)PyArrayDescr_Type.tp_new((PyTypeObject *)&QuadDType, NULL, NULL);
    if (new == NULL) {
        return NULL;
    }

    new->base.elsize = sizeof(__float128);
    new->base.alignment = _Alignof(__float128);
    return new;
}

// Take an python double and put a copy into the array
static int
quad_setitem(QuadDTypeObject *descr, PyObject *obj, char *dataptr)
{
    __float128 val = (__float128)PyFloat_AsDouble(obj);
    memcpy(dataptr, &val, sizeof(__float128));
    return 0;
}

static PyObject *
quad_getitem(QuadDTypeObject *descr, char *dataptr)
{
    __float128 val;
    memcpy(&val, dataptr, sizeof(__float128));

    PyObject *val_obj = PyFloat_FromDouble((double)val);
    if (val_obj == NULL) {
        return NULL;
    }

    // Need to create a new QuadScalar instance here and return that...
    PyObject *res = PyObject_CallFunctionObjArgs((PyObject *)QuadScalar_Type, val_obj, NULL);
    if (res == NULL) {
        return NULL;
    }
    Py_DECREF(val_obj);
    return res;
}

// For two instances of the same dtype, both have the same precision. Return
// self.
static QuadDTypeObject *
common_instance(QuadDTypeObject *self, QuadDTypeObject *other)
{
    return self;
}

// When dtypes are mixed, find a "common" dtype for the two which can hold
// content of both without loss of information. I guess this should return a
// 256-bit float dtype? Since this isn't natively supported by any platform,
// just return another 128-bit float dtype.
static PyArray_DTypeMeta *
common_dtype(PyArray_DTypeMeta *self, PyArray_DTypeMeta *other)
{
    /*
     * Typenum is useful for NumPy, but there it can still be convenient.
     * (New-style user dtypes will probably get -1 as type number...)
     */
    if (other->type_num >= 0 && PyTypeNum_ISNUMBER(other->type_num) &&
        !PyTypeNum_ISCOMPLEX(other->type_num)) {
        // float128 is the biggest natively supported float. Return it in all
        // cases where other is a number (and not complex).
        Py_INCREF(self);
        return self;
    }

    // Revert to object dtype in all other cases.
    Py_INCREF(Py_NotImplemented);
    return (PyArray_DTypeMeta *)Py_NotImplemented;
}

// Expected to have this, and that it does an incref; see NEP42
// Without this you'll get weird memory corruption bugs in the casting code
static QuadDTypeObject *
quaddtype_ensure_canonical(QuadDTypeObject *self)
{
    Py_INCREF(self);
    return self;
}

static PyArray_Descr *
quad_discover_descriptor_from_pyobject(PyArray_DTypeMeta *NPY_UNUSED(cls), PyObject *obj)
{
    if (Py_TYPE(obj) != QuadScalar_Type) {
        PyErr_SetString(PyExc_TypeError, "Can only store QuadScalars in a QuadDType array.");
        return NULL;
    }

    // Get the dtype attribute from the object.
    return (PyArray_Descr *)PyObject_GetAttrString(obj, "dtype");
}

static PyType_Slot QuadDType_Slots[] = {
        {NPY_DT_common_instance, &common_instance},
        {NPY_DT_common_dtype, &common_dtype},
        {NPY_DT_discover_descr_from_pyobject, &quad_discover_descriptor_from_pyobject},
        /* The header is wrong on main :(, so we add 1 */
        {NPY_DT_setitem, &quad_setitem},
        {NPY_DT_getitem, &quad_getitem},
        {NPY_DT_ensure_canonical, &quaddtype_ensure_canonical},
        {0, NULL}};

/*
 * The following defines everything type object related (i.e. not NumPy
 * specific).
 *
 * Note that this function is by default called without any arguments to fetch
 * a default version of the descriptor (in principle at least).  During init
 * we fill in `cls->singleton` though for the dimensionless unit.
 */
static PyObject *
quaddtype_new(PyTypeObject *NPY_UNUSED(cls), PyObject *args, PyObject *kwargs)
{
    return (PyObject *)new_quaddtype_instance();
}

static void
quaddtype_dealloc(QuadDTypeObject *self)
{
    PyArrayDescr_Type.tp_dealloc((PyObject *)self);
}

static PyObject *
quaddtype_repr(QuadDTypeObject *self)
{
    PyObject *res = PyUnicode_FromString("This is a quad (128-bit float) dtype.");
    return res;
}

// These are the basic things that you need to create a Python Type/Class in C.
// However, there is a slight difference here because we create a
// PyArray_DTypeMeta, which is a larger struct than a typical type.
// (This should get a bit nicer eventually with Python >3.11.)
PyArray_DTypeMeta QuadDType = {
        {{
                PyVarObject_HEAD_INIT(NULL, 0).tp_name = "quaddtype.QuadDType",
                .tp_basicsize = sizeof(QuadDTypeObject),
                .tp_new = quaddtype_new,
                .tp_dealloc = (destructor)quaddtype_dealloc,
                .tp_repr = (reprfunc)quaddtype_repr,
                .tp_str = (reprfunc)quaddtype_repr,
        }},
        /* rest, filled in during DTypeMeta initialization */
};

int
init_quad_dtype(void)
{
    // To create our DType, we have to use a "Spec" that tells NumPy how to
    // do it.  You first have to create a static type, but see the note there!
    PyArrayMethod_Spec *casts[] = {
            &QuadToQuadCastSpec,
            NULL,
    };

    PyArrayDTypeMeta_Spec QuadDType_DTypeSpec = {
            .casts = casts,
            .typeobj = QuadScalar_Type,
            .slots = QuadDType_Slots,
    };

    ((PyObject *)&QuadDType)->ob_type = &PyArrayDTypeMeta_Type;
    ((PyTypeObject *)&QuadDType)->tp_base = &PyArrayDescr_Type;
    if (PyType_Ready((PyTypeObject *)&QuadDType) < 0) {
        return -1;
    }

    if (PyArrayInitDTypeMeta_FromSpec(&QuadDType, &QuadDType_DTypeSpec) < 0) {
        return -1;
    }

    QuadDType.singleton = PyArray_GetDefaultDescr(&QuadDType);
    return 0;
}
