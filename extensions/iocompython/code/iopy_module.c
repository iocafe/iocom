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

static struct PyModuleDef spammodule;

PyObject *SpamError;


/* Prototyped for forward referred static functions.
 */



/**
****************************************************************************************************

  @brief Initialize node configuration structure.

  The iotopology_initialize_node_configuration() function initalizes iotopologyNode structure
  and creates mutex to synchronize access to node configuration information.
and initialize it in your module’s initialization function (PyInit_spam()) with an exception object:

  @param   node Pointer to node's network topology configuration to initialize.
  @return  None.

****************************************************************************************************
*/
PyMODINIT_FUNC IOCOMPYTHON_INIT_FUNC (void)
{
    PyObject *m;

Py_Initialize();
//     node->lock = osal_mutex_create();


    m = PyModule_Create(&spammodule);
    if (m == NULL)
        return NULL;

    SpamError = PyErr_NewException(IOCOMPYTHON_NAME ".error", NULL, NULL);
    Py_XINCREF(SpamError);
    if (PyModule_AddObject(m, "error", SpamError) < 0) {
        Py_XDECREF(SpamError);
        Py_CLEAR(SpamError);
        Py_DECREF(m);
        return NULL;
    }

  if (PyType_Ready(&ClassyType) < 0)
    return NULL;

  Py_INCREF(&ClassyType);
  PyModule_AddObject(m, "Classy", (PyObject *)&ClassyType);

    /* Initialize OSAL library for use.
     */
    osal_initialize(OSAL_INIT_NO_LINUX_SIGNAL_INIT);

    return m;
}


/* PyMODINIT_FUNC PyInit_example(void) {
  Py_Initialize();
  PyObject *m = PyModule_Create(&example_definition);

  if (PyType_Ready(&ClassyType) < 0)
    return NULL;

  Py_INCREF(&ClassyType);
  PyModule_AddObject(m, "Classy", (PyObject *)&ClassyType);

  return m;
} */


/**
****************************************************************************************************

  @brief Release all memory allocated for node configuration structure.

  The iotopology_release_node_configuration() function releases all memory allocated for
  IO node configuration structure.

  @param   node Pointer to node's network topology configuration to release.
  @return  None.

****************************************************************************************************
*/
void xxx_release()
{
    osal_shutdown();
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
        PyErr_SetString(SpamError, "System command failed");
        return NULL;
    }
    return PyLong_FromLong(sts);
}

/**
****************************************************************************************************

  @brief Set application name and version.

  The iotopology_set_application_name() function stores application name and version into node
  configuration. Application name and version are used to identify the software which the
  IO device or controller runs.

  @param   node Pointer to node's network topology configuration.
  @param   app_name Name of the application.
  @param   app_version Application version string.
  @return  None.

****************************************************************************************************
*/
/* void iotopology_set_application_name(void)
{
} */


static PyMethodDef SpamMethods[] = {
    {"system",  spam_system, METH_VARARGS,
     "Execute a shell command."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef spammodule = {
    PyModuleDef_HEAD_INIT,
    IOCOMPYTHON_NAME,   /* name of module */
    NULL, // spam_doc, /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    SpamMethods
};
