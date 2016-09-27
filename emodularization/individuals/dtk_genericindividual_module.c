#include <Python.h>
#include <iostream>

#include "Individual.h"

Kernel::IndividualHuman * person = nullptr;
Configuration * configStubJson = nullptr;

static void initInd( bool dr = false )
{
    /*if( dr == true )
    {
        std::cout << "Doing get-schema-style init." << std::endl;
        Kernel::JsonConfigurable::_dryrun = dr;
        Kernel::IndividualHuman::InitializeStatics( nullptr );
        Kernel::JsonConfigurable::_dryrun = false;
    }
    else*/ if( configStubJson == nullptr )
    {
        configStubJson = Configuration::Load("gi.json");
        Kernel::JsonConfigurable::_useDefaults = true;
        Kernel::IndividualHuman::InitializeStatics( configStubJson );
        Kernel::JsonConfigurable::_useDefaults = false;
    }
}


static PyObject*
create(PyObject* self, PyObject* args)
{
    //const char* name;

    //if (!PyArg_ParseTuple(args, "s", &name))
        //return NULL;

    //printf("Hello %s!\n", name);
    person = Kernel::IndividualHuman::CreateHuman( nullptr, Kernel::suids::nil_suid(), 1.0f, 365.0f, 0, 0 );
    initInd();

    Py_RETURN_NONE;
}

static PyObject*
update(PyObject* self, PyObject* args)
{
    /*const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Bye-bye %s!\n", name);*/
    person->Update( 0.0f, 1.0f );

    Py_RETURN_NONE;
}

// Simualtion class is a friend which is necessary for calling Configure
namespace Kernel {
    class Simulation
    {
        public:
            static std::string result;
            Simulation()
            {
                Kernel::JsonConfigurable::_dryrun = true;
                Kernel::IndividualHumanConfig adam;
                adam.Configure( nullptr ); // protected
                auto schema = adam.GetSchema();
                std::ostringstream schema_ostream;
                json::Writer::Write( schema, schema_ostream );
                //std::cout << schema_ostream.str() << std::endl;
                result = schema_ostream.str();
                Kernel::JsonConfigurable::_dryrun = false;
            }
    };
    std::string Simulation::result = "";
}

static PyObject*
getSchema(PyObject* self, PyObject* args)
{
    bool ret = false;
    Kernel::Simulation ti;
    //std::cout << ti.result.c_str() << std::endl;
    return Py_BuildValue("s", ti.result.c_str() );;
}

static PyMethodDef GenericIndividualMethods[] =
{
     {"create", create, METH_VARARGS, "Create somebody."},
     {"update", update, METH_VARARGS, "Update somebody."},
     {"get_schema", getSchema, METH_VARARGS, "Update."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdtk_genericindividual(void)
{
     (void) Py_InitModule("dtk_genericindividual", GenericIndividualMethods);
}
