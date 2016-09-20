#include <Python.h>

#include "IdmDateTime.h"

Kernel::IdmDateTime _instance;

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

static PyObject*
getBaseYear(PyObject* self, PyObject* args)
{
    float base_year = _instance.getBaseYear();
    
    return Py_BuildValue("f", base_year);;
}

static PyObject*
setBaseYear(PyObject* self, PyObject* args)
{
    float base_year = 0.0f;

    if (!PyArg_ParseTuple(args, "f", &base_year))
        return NULL;

    _instance.setBaseYear( base_year );
    
    Py_RETURN_NONE;
}

static PyMethodDef DtkDateTimeMethods[] =
{
     {"create", create_individual, METH_VARARGS, "Create new object."},
     {"update", update, METH_VARARGS, "Update."},
     {"get_base_year", getBaseYear, METH_VARARGS, "Get the base year (if set). Returns 0 if not."},
     {"set_base_year", setBaseYear, METH_VARARGS, "Sets the base year."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdtk_datetime(void)
{
     (void) Py_InitModule("dtk_datetime", DtkDateTimeMethods);
}
