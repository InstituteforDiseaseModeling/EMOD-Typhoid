#include <Python.h>
#include <iostream>

#include "TyphoidVaccine.h"
#include "TyphoidInterventionsContainer.h"
#include "IndividualEventContext.h"
#include "IndividualEnvironmental.h"
#include "IndividualTyphoid.h"
#include "WaningEffect.h"

using namespace Kernel;

//Kernel::WaningEffectFactory fact;

Kernel::TyphoidVaccine _instance;
std::vector<Kernel::TyphoidVaccine*> _batch; // array of pointers

Configuration * configStubJson = nullptr;

static PyObject *my_callback = NULL;

static PyObject *
my_set_callback(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp))
    {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        } 
        Py_XINCREF(temp);         /* Add a reference to new callback */ 
        Py_XDECREF(my_callback);  /* Dispose of previous callback */ 
        my_callback = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

static void initTV( bool dr = false )
{
    if( dr == true )
    {
        Kernel::JsonConfigurable::_dryrun = dr;
        _instance.Configure( nullptr  );
        Kernel::JsonConfigurable::_dryrun = false;
    }
    else if( configStubJson == nullptr )
    {
        configStubJson = Configuration::Load("tv.json");
        Kernel::JsonConfigurable::_useDefaults = true;
        _instance.Configure( configStubJson  );
    }
}

//class StubTyphoidIndividual : public IIndividualHumanInterventionsContext, public ITyphoidVaccineEffectsApply, public IInterventionConsumer
class StubTyphoidInterventionsContainer : //public IndividualHumanEnvironmental, public IIndividualHumanTyphoid
    public InterventionsContainer, public ITyphoidVaccineEffectsApply
{
    public:
        StubTyphoidInterventionsContainer  () {}
        ~StubTyphoidInterventionsContainer  ()
        {
            std::cout << "StubTyphoidInterventionsContainer DTOR." << std::endl;
        }
        
        virtual int32_t AddRef() {}
        virtual int32_t Release() {}

        Kernel::QueryResult QueryInterface( iid_t iid, void **ppinstance )
        {
            assert(ppinstance);

            if ( !ppinstance )
                return e_NULL_POINTER;

            ISupports* foundInterface;

            if ( iid == GET_IID(ITyphoidVaccineEffectsApply)) 
                foundInterface = static_cast<ITyphoidVaccineEffectsApply*>(this);
            // -->> add support for other I*Consumer interfaces here <<--      
            else if ( iid == GET_IID(ISupports)) 
                foundInterface = static_cast<ISupports*>(static_cast<ITyphoidVaccineEffectsApply*>(this));
            else if ( iid == GET_IID(IInterventionConsumer)) 
                foundInterface = static_cast<ISupports*>(static_cast<IInterventionConsumer*>(this));

            else
                foundInterface = 0;

            QueryResult status;
            if ( !foundInterface )
                status = e_NOINTERFACE;
            else
            {
                //foundInterface->AddRef();           // not implementing this yet!
                status = s_OK;
            }

            *ppinstance = foundInterface;
            return status;
        }

        // This is so we can pass a faux-man
        virtual void SetContextTo(IIndividualHumanContext *context) override { std::cout << __FUNCTION__ << std::endl; }
        virtual IIndividualHumanContext* GetParent() override {  std::cout << __FUNCTION__ << std::endl; return nullptr; }
        virtual std::list<IDistributableIntervention*> GetInterventionsByType(const std::string &type_name) override { std::cout << __FUNCTION__ << std::endl; }
        virtual std::list<IDistributableIntervention*> GetInterventionsByName(const std::string &intervention_name) override { std::cout << __FUNCTION__ << std::endl; }
        virtual std::list<void*> GetInterventionsByInterface( iid_t iid ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void PurgeExisting( const std::string &iv_name ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual bool ContainsExisting( const std::string &iv_name ) override {  std::cout << __FUNCTION__ << std::endl; return false; }
        virtual bool GiveIntervention( IDistributableIntervention * pIV ) override
        {
            std::cout << "Intervention distributed to individual. Call py callback here?" << std::endl;
        }

        virtual void Test() 
        {
            std::cout << __FUNCTION__ << std::endl;
        }

        virtual void ApplyReducedSheddingEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) override
        {
            std::cout << __FUNCTION__ << ": rate = " << rate << ", route = " << route << std::endl;
        }

        virtual void ApplyReducedDoseEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) override
        {
            std::cout << __FUNCTION__ << ": rate = " << rate << ", route = " << route << std::endl;
        }

        virtual void ApplyReducedNumberExposuresEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) override
        {
            std::cout << __FUNCTION__ << ": rate = " << rate << ", route = " << route << std::endl;
        }
};
StubTyphoidInterventionsContainer man;
std::vector<StubTyphoidInterventionsContainer> _pop;

static PyObject*
distribute(PyObject* self, PyObject* args)
{
    bool ret = false;

    initTV();
    ret = _instance.Distribute( &man, nullptr );
    return Py_BuildValue( "b", ret );;
}

static PyObject*
distributeBatch(PyObject* self, PyObject* args)
{
    bool ret = false;

    _pop.resize( _batch.size() );
    for( int idx = 0; idx < _batch.size(); idx ++ )
    {
        auto tyvac = _batch[ idx ];
        auto &man = _pop[ idx ];
        ret = tyvac->Distribute( &man, nullptr );
    }

    return Py_BuildValue( "b", ret );;
}

static PyObject*
update(PyObject* self, PyObject* args)
{
    float dt;

    if( !PyArg_ParseTuple(args, "f", &dt ) )
        return NULL;

    _instance.Update( dt );

    return Py_BuildValue("b", true );
}

static PyObject*
updateBatch(PyObject* self, PyObject* args)
{
    float dt;

    if( !PyArg_ParseTuple(args, "f", &dt ) )
        return NULL;

    for( int idx = 0; idx < _batch.size(); idx ++ )
    {
        auto tyvac = _batch[ idx ];
        tyvac->Update( dt );
    }

    return Py_BuildValue("b", true );
}

static PyObject*
createBatch(PyObject* self, PyObject* args)
{
    unsigned int number;

    if( !PyArg_ParseTuple(args, "I", &number ) )
    {
        std::cout << "That didn't work: " << __FUNCTION__ << std::endl;
        return NULL;
    }
    std::cout << "number = " << number << std::endl;

    _batch.resize( number );
    auto configStubJson = Configuration::Load("tv.json");
    Kernel::JsonConfigurable::_useDefaults = true;
    for( int idx = 0; idx<number; ++idx )
    {
        auto * TV = new Kernel::TyphoidVaccine();
        TV->Configure( configStubJson  );
        _batch[ idx ] = TV;
    }
    Kernel::JsonConfigurable::_useDefaults = false;

    return Py_BuildValue("b", true );
}

static PyObject*
getSchema(PyObject* self, PyObject* args)
{
    bool ret = false;

    initTV( true );
    auto schema = _instance.GetSchema();
    std::ostringstream schema_ostream;
    json::Writer::Write( schema, schema_ostream );
    return Py_BuildValue("s", schema_ostream.str().c_str() );;
}

static PyMethodDef DtktyvacMethods[] =
{
     {"create_batch", createBatch, METH_VARARGS, "Create a list of N tyvacs."},
     {"distribute_batch", distributeBatch, METH_VARARGS, "Distribute the N tyvacs to N individuals."},
     {"update_batch", updateBatch, METH_VARARGS, "Update the N tyvacs."},
     {"distribute", distribute, METH_VARARGS, "Distribute."},
     {"update", update, METH_VARARGS, "Update."},
     {"my_set_callback", my_set_callback, METH_VARARGS, "Update."},
     {"get_schema", getSchema, METH_VARARGS, "Update."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdtk_tyvac(void)
{
     (void) Py_InitModule("dtk_tyvac", DtktyvacMethods);
}


