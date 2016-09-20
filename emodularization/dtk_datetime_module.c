#include <Python.h>


IdmDateTime _instance;

static PyObject*
create_individual(PyObject* self, PyObject* args)
{
    const char* name;

    printf( "%s not implemented yet.\n", __FUNCTION__ );
    /*if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Hello %s!\n", name);*/

    Py_RETURN_NONE;
}

static PyObject*
update(PyObject* self, PyObject* args)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Bye-bye %s!\n", name);

    Py_RETURN_NONE;
}

static PyMethodDef DtkDateTimeMethods[] =
{
     {"create", create_individual, METH_VARARGS, "Create new object."},
     {"update", update, METH_VARARGS, "Update."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdtk_datetime(void)
{
     (void) Py_InitModule("dtk_datetime", DtkDateTimeMethods);
}
