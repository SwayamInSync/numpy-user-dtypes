#ifndef _NPY_DTYPE_H
#define _NPY_DTYPE_H

#include<Python.h>
#include<numpy/ndarraytypes.h>
#include<numpy/dtype_api.h>

typedef struct
{
    PyArray_Descr base;
    
} QuadDTypeObject;

extern PyArray_DTypeMeta QuadDType;
extern PyTypeObject *QuadScalar_Type;

QuadDTypeObject * new_quaddtype_instance(void);

int init_quad_dtype(void);

#endif