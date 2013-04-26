#include <Python.h>

#include <stdio.h>

#include "screen.h"

static PyObject *screen_scopen(PyObject *self, PyObject *args)
{
	platformOpenScreen();
    return Py_BuildValue("i", 9999);
}

static PyObject *screen_scclose(PyObject *self, PyObject *args)
{
	Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *screen_scprint(PyObject *self, PyObject *args)
{
	Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *screen_scread(PyObject *self, PyObject *args)
{
    getchar();

	Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef screenMethods[] =
{
    {"scopen", screen_scopen, METH_VARARGS, "Open screen"},
    {"scclose", screen_scclose, METH_VARARGS, "Close screen"},
    {"scprint", screen_scprint, METH_VARARGS, "Prints string to current screen"},
    {"scread", screen_scread, METH_VARARGS, "Reads string form user input"},
    {NULL, NULL, 0, NULL} // Sentinel
};

PyMODINIT_FUNC initscreen()
{
    PyObject *m;

    m = Py_InitModule("screen", screenMethods);
    if (m == NULL)
        return;

    // SpamError = PyErr_NewException("spam.error", NULL, NULL);
    // Py_INCREF(SpamError);
    // PyModule_AddObject(m, "error", SpamError);
}
