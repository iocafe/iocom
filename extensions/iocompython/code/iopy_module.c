/**

  @file    iopy_module.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    22.10.2019

  Explose Python module interface.

  Copyright 2018 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "extensions/iocompython/iocompython.h"


PyObject *iocomError;

/* Counter for iocom_python_initialize() and iocom_python_release() calls.
 */
static os_int module_init_count;


/**
****************************************************************************************************

  @brief Python module initialization function.

  X...

  @return  None.

****************************************************************************************************
*/
PyMODINIT_FUNC IOCOMPYTHON_INIT_FUNC (void)
{
    PyObject *m;


    Py_Initialize(); // ????????

    m = PyModule_Create(&iocompythonmodule);
    if (m == NULL) return NULL;

    iocomError = PyErr_NewException(IOCOMPYTHON_NAME ".error", NULL, NULL);
    Py_XINCREF(iocomError);
    if (PyModule_AddObject(m, "error", iocomError) < 0)
    {
        Py_XDECREF(iocomError);
        Py_CLEAR(iocomError);
        Py_DECREF(m);
        return NULL;
    }

    if (PyType_Ready(&RootType) < 0)
        return NULL;

    if (PyType_Ready(&MemoryBlockType) < 0)
        return NULL;

    if (PyType_Ready(&ConnectionType) < 0)
        return NULL;

    Py_INCREF(&RootType);
    PyModule_AddObject(m, "Root", (PyObject *)&RootType);
    Py_INCREF(&MemoryBlockType);
    PyModule_AddObject(m, "MemoryBlock", (PyObject *)&MemoryBlockType);
    Py_INCREF(&ConnectionType);
    PyModule_AddObject(m, "Connection", (PyObject *)&ConnectionType);

    module_init_count = 0;

    return m;
}



static PyObject *
spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    if (sts < 0) {
        PyErr_SetString(iocomError, "System command failed");
        return NULL;
    }
    return PyLong_FromLong(sts);
}


/**
****************************************************************************************************

  @brief Initialize operating system abstraction layer and communication transport libraries.

  The iocom_python_initialize() function...
  @return  None.

****************************************************************************************************
*/
void iocom_python_initialize(void)
{
    if (module_init_count++) return;

    osal_initialize(OSAL_INIT_NO_LINUX_SIGNAL_INIT);

#if OSAL_TLS_SUPPORT
    osal_tls_initialize(OS_NULL, 0, OS_NULL);
#else
  #if OSAL_SOCKET_SUPPORT
    osal_socket_initialize(OS_NULL, 0);
  #endif
#endif

#if OSAL_SERIAL_SUPPORT
    osal_serial_initialize();
#endif

#if OSAL_BLUETOOTH_SUPPORT
    osal_bluetooth_initialize();
#endif
}


/**
****************************************************************************************************

  @brief Shut down operating system abstraction layer and communication transport libraries.

  The iocom_python_release() function...
  @return  None.

****************************************************************************************************
*/
void iocom_python_release(void)
{
    if (--module_init_count) return;

#if OSAL_TLS_SUPPORT
    osal_tls_shutdown();
#else
  #if OSAL_SOCKET_SUPPORT
    osal_socket_shutdown();
  #endif
#endif

#if OSAL_SERIAL_SUPPORT
    osal_serial_shutdown();
#endif

#if OSAL_BLUETOOTH_SUPPORT
    osal_bluetooth_shutdown();
#endif
}


/**
****************************************************************************************************
  Module's global functions.
****************************************************************************************************
*/
static PyMethodDef iocomPythonMethods[] = {
    {"system",  spam_system, METH_VARARGS, "Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


/**
****************************************************************************************************
  Python module definition.
****************************************************************************************************
*/
struct PyModuleDef iocompythonmodule = {
    PyModuleDef_HEAD_INIT,
    IOCOMPYTHON_NAME,   /* name of module */
    NULL,               /* module documentation, may be NULL */
    -1,                 /* size of per-interpreter state of the module,
                           or -1 if the module keeps state in global variables. */
    iocomPythonMethods
};
