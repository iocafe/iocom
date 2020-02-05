/**

  @file    iopy_module.c
  @brief   Python wrapper for the IOCOM library.
  @author  Pekka Lehtikoski
  @version 1.0
  @date    8.1.2020

  Explose Python module interface.

  Copyright 2020 Pekka Lehtikoski. This file is part of the iocom project and shall only be used,
  modified, and distributed under the terms of the project licensing. By continuing to use, modify,
  or distribute this file you indicate that you have read the license and understand and accept
  it fully.

****************************************************************************************************
*/
#include "iocompython.h"


PyObject *iocomError;

/* Counter for iocom_python_initialize() and iocom_python_release() calls.
 */
static os_int module_init_count;

/* Forward referred static functions.
 */
os_char *iocom_get_security_param(
    os_char *name,
    os_char **dst,
    os_memsz *dst_sz,
    const char *security);


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

    if (PyType_Ready(&EndPointType) < 0)
        return NULL;

    if (PyType_Ready(&SignalType) < 0)
        return NULL;

    if (PyType_Ready(&StreamType) < 0)
        return NULL;

    Py_INCREF(&RootType);
    PyModule_AddObject(m, "Root", (PyObject *)&RootType);
    Py_INCREF(&MemoryBlockType);
    PyModule_AddObject(m, "MemoryBlock", (PyObject *)&MemoryBlockType);
    Py_INCREF(&ConnectionType);
    PyModule_AddObject(m, "Connection", (PyObject *)&ConnectionType);
    Py_INCREF(&EndPointType);
    PyModule_AddObject(m, "EndPoint", (PyObject *)&EndPointType);
    Py_INCREF(&SignalType);
    PyModule_AddObject(m, "Signal", (PyObject *)&SignalType);
    Py_INCREF(&StreamType);
    PyModule_AddObject(m, "Stream", (PyObject *)&StreamType);

    module_init_count = 0;

    return m;
}


/**
****************************************************************************************************

  @brief Initialize operating system abstraction layer and communication transport libraries.

  The iocom_python_initialize() function...
  @param   security Pointer to TLS configuration parameter structure.
  @return  None.

****************************************************************************************************
*/
void iocom_python_initialize(const char *security)
{
    osalSecurityConfig secprm;
    os_char buf[256], *dst;
    os_memsz dst_sz;

    if (module_init_count++) return;

    osal_initialize(OSAL_INIT_NO_LINUX_SIGNAL_INIT);

#if OSAL_TLS_SUPPORT
    os_memclear(&secprm, sizeof(secprm));
    buf[0] = '\0';

    dst = buf;
    dst_sz = sizeof(buf);
    secprm.certs_dir = iocom_get_security_param("certdir", &dst, &dst_sz, security);
    secprm.server_cert_file = iocom_get_security_param("certfile", &dst, &dst_sz, security);
    secprm.server_key_file = iocom_get_security_param("keyfile", &dst, &dst_sz, security);
    secprm.root_cert_file = iocom_get_security_param("rootca", &dst, &dst_sz, security);
    secprm.client_cert_chain_file = iocom_get_security_param("certchainfile", &dst, &dst_sz, security);
    osal_tls_initialize(OS_NULL, 0, OS_NULL, 0, &secprm);
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

  @brief Copy a string parameter value out of "security" string.

  The security parameter string is something like "certfile=bob.crt,keyfile=bob.key". The
  iocom_get_security_param finds a parameter by name from parameter string, makes a '\0'
  copy of the parameter value string into buffer (dst, dst_sz) and returns pointer returns
  pointer to this copy.

  @param   name Parameter name to search for.
  @param   dst Pointer to buffer position pointer where to copy the next value string.
           Modified by this function.
  @param   dst_sz Pointer to bumber of bytes left in the buffer. Modified by this function.
  @return  Pointer to '\0' terminated copy of value string (in buffer), or OS_NULL if the
           parameter with name was not in security string.

****************************************************************************************************
*/
os_char *iocom_get_security_param(
    os_char *name,
    os_char **dst,
    os_memsz *dst_sz,
    const char *security)
{
    const os_char *p;
    os_char *e;
    os_memsz n_chars;

    p = osal_str_get_item_value(security, name, &n_chars, OSAL_STRING_DEFAULT);
    if (p == OS_NULL) return OS_NULL;

    if (n_chars >= *dst_sz)
    {
        osal_debug_error("out of security configuration string buffer");
        return OS_NULL;
    }

    e = *dst;
    os_memcpy(e, p, n_chars);
    e[n_chars] = '\0';

    *dst_sz -= n_chars + 1;
    *dst += n_chars + 1;
    return e;
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

    osal_shutdown();
}


/**
****************************************************************************************************
  Module's global functions.
****************************************************************************************************
*/
static PyMethodDef iocomPythonMethods[] = {
    {"json2bin",   (PyCFunction)iocom_python_json2bin, METH_VARARGS, "Convert JSON from text to packed binary format."},
    {"bin2json",   (PyCFunction)iocom_python_bin2json, METH_VARARGS, "Convert JSON from packed binary format to text."},
    {"get_secret", (PyCFunction)iocom_python_get_secret, METH_NOARGS, "Get security secret"},
    {"get_password", (PyCFunction)iocom_python_get_password, METH_NOARGS, "Get automatically generated password"},
    {"hash_password", (PyCFunction)iocom_python_hash_password, METH_VARARGS, "Hash password (run SHA-256 hash on password)"},
    {"forget_secret", (PyCFunction)iocom_python_forget_secret, METH_NOARGS, "Forget the secret (and password)"},

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
