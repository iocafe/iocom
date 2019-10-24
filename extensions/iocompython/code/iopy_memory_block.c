/**

  @file    iopy_memory_block.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iocompython/iocompython.h"


/**
****************************************************************************************************

  @brief Constructor.

  The MemoryBlock_new function generates a new root object.
  @return  Pointer to the new Python object.

****************************************************************************************************
*/
static PyObject *MemoryBlock_new(
    PyTypeObject *type,
    PyObject *args,
    PyObject *kwds)
{
    MemoryBlock *self;

    self = (MemoryBlock *)type->tp_alloc(type, 0);
    if (self != NULL) {
    self->number = 8;
    }

    PySys_WriteStdout("new\n");

    return (PyObject *)self;
}


/**
****************************************************************************************************

  @brief Destructor.

  The MemoryBlock_dealloc function releases all resources allocated for the root object. This function
  gets called when reference count to puthon object drops to zero.
  @param   self Pointer to the python object.
  @return  None.

****************************************************************************************************
*/
static void MemoryBlock_dealloc(
    MemoryBlock *self)
{
    Py_TYPE(self)->tp_free((PyObject *)self);

    PySys_WriteStdout("del\n");
}


/**
****************************************************************************************************

  @brief Initialize.

  I do not think this is needed

  The MemoryBlock_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static int MemoryBlock_init(
    MemoryBlock *self,
    PyObject *args,
    PyObject *kwds)
{
    self->number = 1;

    PySys_WriteStdout("init\n");
    return 0;
}


/**
****************************************************************************************************

  @brief Initialize.

  X...

  The MemoryBlock_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *MemoryBlock_miami(
    MemoryBlock *self)
{
    if (self->number > 1)
    self->number /= 2;

    PySys_WriteStdout("in miami\n");
    return PyLong_FromLong((long)self->number);
}


/**
****************************************************************************************************

  @brief Initialize.

  X...

  The MemoryBlock_init function initializes an object.
  @param   self Pointer to the python object.
  @return  ?.

****************************************************************************************************
*/
static PyObject *MemoryBlock_delete(
    MemoryBlock *self)
{
    if (self->number < 1024 * 1024)
    self->number *= 2;

    PySys_WriteStdout("in newest york\n");
    return PyLong_FromLong((long)self->number);
}


/**
****************************************************************************************************
  Member variables.
****************************************************************************************************
*/
static PyMemberDef MemoryBlock_members[] = {
    {(char*)"number", T_INT, offsetof(MemoryBlock, number), 0, (char*)"classy number"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Member functions.
****************************************************************************************************
*/
static PyMethodDef MemoryBlock_methods[] = {
    {"miami", (PyCFunction)MemoryBlock_miami, METH_NOARGS, "Divides number by 2"},
    {"delete", (PyCFunction)MemoryBlock_delete, METH_NOARGS, "Deletes underlying memory block"},
    {NULL} /* Sentinel */
};


/**
****************************************************************************************************
  Class type setup.
****************************************************************************************************
*/
PyTypeObject MemoryBlockType = {
    PyVarObject_HEAD_INIT(NULL, 0) IOCOMPYTHON_NAME ".MemoryBlock",  /* tp_name */
    sizeof(MemoryBlock),                           /* tp_basicsize */
    0,                                        /* tp_itemsize */
    (destructor)MemoryBlock_dealloc,               /* tp_dealloc */
    0,                                        /* tp_print */
    0,                                        /* tp_getattr */
    0,                                        /* tp_setattr */
    0,                                        /* tp_reserved */
    0,                                        /* tp_repr */
    0,                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "MemoryBlock objects",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    MemoryBlock_methods,                           /* tp_methods */
    MemoryBlock_members,                           /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)MemoryBlock_init,                    /* tp_init */
    0,                                        /* tp_alloc */
    MemoryBlock_new,                               /* tp_new */
};
