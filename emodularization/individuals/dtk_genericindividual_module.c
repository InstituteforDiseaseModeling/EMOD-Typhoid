#include <Python.h>
#include <iostream>
#include "RANDOM.h"

#include "Environment.h"
#include "SimulationConfig.h"
#include "NodeEventContext.h"
#include "Individual.h"
#include "INodeContext.h"
#include "Properties.h"

Kernel::IndividualHuman * person = nullptr;
Configuration * configStubJson = nullptr;
static PyObject *my_callback = NULL;

using namespace Kernel;

//
// Initialize the rng in the Environment.
//
void
pyMathFuncInit()
{
    if( Environment::getInstance()->RNG == nullptr )
    {
        unsigned int randomseed[2];
        randomseed[0] = 0;
        randomseed[1] = 0;
        auto rng = new PSEUDO_DES(*reinterpret_cast<uint32_t*>(randomseed));
        const_cast<Environment*>(Environment::getInstance())->RNG = rng;
    }
}

// 
// We'll need a stub node object as the individual's parent. Most functions are empty.
//
class StubNode : public INodeContext
{
    public:
        StubNode()
        {
            //IPFactory::CreateFactory();
            pyMathFuncInit();
        }

        virtual int32_t AddRef() {}
        virtual int32_t Release() {}

        Kernel::QueryResult QueryInterface( iid_t iid, void **ppinstance )
        {
            assert(ppinstance);

            if ( !ppinstance )
                return e_NULL_POINTER;

            ISupports* foundInterface;

            /*if ( iid == GET_IID(ISporozoiteChallengeConsumer)) 
                foundInterface = static_cast<ISporozoiteChallengeConsumer*>(this);*/
            // -->> add support for other I*Consumer interfaces here <<--      
            //else 
            if ( iid == GET_IID(ISupports)) 
                foundInterface = static_cast<ISupports*>((this));
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

        // This is so we can pass a faux-node
        virtual void VisitIndividuals( INodeEventContext::individual_visit_function_t func) { std::cout << __FUNCTION__ << std::endl; }
        virtual int VisitIndividuals(IVisitIndividual* pIndividualVisitImpl, int limit = -1) { std::cout << __FUNCTION__ << std::endl; } 
        virtual const NodeDemographics& GetDemographics() { std::cout << __FUNCTION__ << std::endl; }
        virtual bool GetUrban() const { std::cout << __FUNCTION__ << std::endl; }
        virtual IdmDateTime GetTime() const { std::cout << __FUNCTION__ << std::endl; }
        virtual void UpdateInterventions(float = 0.0f) { std::cout << __FUNCTION__ << std::endl; } 
        virtual void RegisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, INodeEventContext::TravelEventType type) { std::cout << __FUNCTION__ << std::endl; }
        virtual void UnregisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, INodeEventContext::TravelEventType type) { std::cout << __FUNCTION__ << std::endl; } 
        virtual const suids::suid & GetId() const { std::cout << __FUNCTION__ << std::endl; }
        virtual void SetContextTo(INodeContext* context) { std::cout << __FUNCTION__ << std::endl; }
        virtual std::list<INodeDistributableIntervention*> GetInterventionsByType(const std::string& type_name) { std::cout << __FUNCTION__ << std::endl; }
        virtual void PurgeExisting( const std::string& iv_name ) { std::cout << __FUNCTION__ << std::endl; } 
        virtual bool IsInPolygon(float* vertex_coords, int numcoords) { std::cout << __FUNCTION__ << std::endl; }
        virtual bool IsInExternalIdSet( const tNodeIdList& nodelist ) { std::cout << __FUNCTION__ << std::endl; }
        virtual ::RANDOMBASE* GetRng() { std::cout << __FUNCTION__ << std::endl; }
        virtual INodeContext* GetNodeContext() { std::cout << __FUNCTION__ << std::endl; } 
        virtual int GetIndividualHumanCount() const { std::cout << __FUNCTION__ << std::endl; }
        virtual ExternalNodeId_t GetExternalId() const { std::cout << __FUNCTION__ << std::endl; }


        virtual ISimulationContext* GetParent() override { std::cout << __FUNCTION__ << std::endl; }
        virtual suids::suid GetSuid() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual void SetContextTo( ISimulationContext* ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void SetMonteCarloParameters(float indsamplerate =.05, int nummininf = 0) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void SetParameters(NodeDemographicsFactory *demographics_factory, ClimateFactory *climate_factory) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void PopulateFromDemographics() override { std::cout << __FUNCTION__ << std::endl; }
        virtual suids::suid GetNextInfectionSuid() override {
            std::cout << __FUNCTION__ << std::endl;
            return Kernel::suids::nil_suid();
        }
        virtual void Update(float dt) override { std::cout << __FUNCTION__ << std::endl; }
        virtual IIndividualHuman* processImmigratingIndividual( IIndividualHuman* ) override { return nullptr; }
        virtual void DepositFromIndividual(StrainIdentity* strain_IDs, float contagion_quantity, const TransmissionGroupMembership_t* individual) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void GetGroupMembershipForIndividual(const RouteList_t& route, tProperties* properties, TransmissionGroupMembership_t* membershipOut ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual void UpdateTransmissionGroupPopulation(const TransmissionGroupMembership_t* membership, float size_changes,float mc_weight) override { std::cout << __FUNCTION__ << std::endl; }
        virtual float GetTotalContagion(const TransmissionGroupMembership_t* membership) override { std::cout << __FUNCTION__ << std::endl; }
        virtual const RouteList_t& GetTransmissionRoutes( ) const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float getSinusoidalCorrection(float sinusoidal_amplitude, float sinusoidal_phase) const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float getBoxcarCorrection(float boxcar_amplitude, float boxcar_start_time, float boxcar_end_time) const override { std::cout << __FUNCTION__ << std::endl; }
        virtual act_prob_vec_t DiscreteGetTotalContagion(const TransmissionGroupMembership_t* membership) override { std::cout << __FUNCTION__ << std::endl; }
        virtual IMigrationInfo* GetMigrationInfo() override { std::cout << __FUNCTION__ << std::endl; }
        virtual const NodeDemographics* GetDemographics() const override { return nullptr; }
        virtual const NodeDemographicsDistribution* GetDemographicsDistribution(std::string) const override { return nullptr; }
        virtual float       GetInfected()      const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float       GetStatPop()       const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float       GetBirths()        const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float       GetCampaignCost()  const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float       GetInfectivity()   const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float       GetInfectionRate() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float       GetSusceptDynamicScaling() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual const Climate* GetLocalWeather() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual long int GetPossibleMothers()  const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float GetMeanAgeInfection()    const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float GetLatitudeDegrees() override { std::cout << __FUNCTION__ << std::endl; }
        virtual float GetLongitudeDegrees() override { std::cout << __FUNCTION__ << std::endl; }
        virtual ExternalNodeId_t GetExternalID() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual INodeEventContext* GetEventContext() override { return nullptr; }
        virtual void AddEventsFromOtherNodes( const std::vector<std::string>& rEventNameList ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual bool IsEveryoneHome() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual float GetBasePopulationScaleFactor() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual ProbabilityNumber GetProbMaternalTransmission() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual void SetupMigration( IMigrationInfoFactory * migration_factory, MigrationStructure::Enum ms, const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap ) override { std::cout << __FUNCTION__ << std::endl; }
        virtual std::vector<bool> GetMigrationTypeEnabledFromDemographics() const override { std::cout << __FUNCTION__ << std::endl; }
        virtual void SetWaitingForFamilyTrip( suids::suid migrationDestination, MigrationType::Enum migrationType, float timeUntilTrip, float timeAtDestination, bool isDestinationNewHome ) override { std::cout << __FUNCTION__ << std::endl; }
        // We want to be able to infect someone but not with existing Tx groups. Call into py layer here and get a bool back
        // True to infect, False to leave alone.
        virtual void ExposeIndividual(IInfectable* candidate, const TransmissionGroupMembership_t* individual, float dt) override
        {
            //transmissionGroups->ExposeToContagion(candidate, individual, dt);
            std::cout << __FUNCTION__ << std::endl;
            PyObject *arglist = Py_BuildValue("(s,f)", "expose", 1.0 );
            PyObject *retVal = PyObject_CallObject(my_callback, arglist);
            bool infect = false;
            infect = PyInt_AsLong( retVal );
            if( infect )
            {
                IInfectionAcquirable* ind = nullptr;
                candidate->QueryInterface( GET_IID( IInfectionAcquirable ), &ind );
                ind->AcquireNewInfection();
            }
            Py_DECREF(arglist);
        }
};

// Declare signle node here
StubNode node;


// Boiler-plate callback setting function that will be put in common file.
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

// Json-configure & Initialize a (single) individual
// Use json file on disk.
static void initInd( bool dr = false )
{
    person = Kernel::IndividualHuman::CreateHuman( &node, Kernel::suids::nil_suid(), 1.0f, 365.0f, 0, 0 );
    if( configStubJson == nullptr )
    {
        configStubJson = Configuration::Load("gi.json");
        std::cout << "configStubJson initialized from gi.json." << std::endl;
        Kernel::JsonConfigurable::_useDefaults = true;
        Kernel::IndividualHuman::InitializeStatics( configStubJson );
        std::cout << "Initialized Statics from gi.json." << std::endl;
        Kernel::JsonConfigurable::_useDefaults = false; 
        person->SetParameters( &node, 0.0f, 1.0f, 0.0f, 0.0f );
    }
}


// create individual human 
static PyObject*
create(PyObject* self, PyObject* args)
{
    initInd();
    Py_RETURN_NONE;
}

//
// Some functions to interact with the human
//
// update individual (hard-coded time values)
static PyObject*
update(PyObject* self, PyObject* args)
{
    person->Update( 0.0f, 1.0f );

    Py_RETURN_NONE;
}

static PyObject*
getAge(PyObject* self, PyObject* args)
{
    auto age = person->GetAge();
    return Py_BuildValue("f", age );
}

static PyObject*
isInfected(PyObject* self, PyObject* args)
{
    bool inf_status = person->IsInfected();
    return Py_BuildValue("b", inf_status );
}

static PyObject*
getImmunity(PyObject* self, PyObject* args)
{
    float imm = person->GetAcquisitionImmunity();
    return Py_BuildValue("f", imm );
}

// Supporting GetSchema here
//
// Simulation class is a friend which is necessary for calling Configure
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
    return Py_BuildValue("s", ti.result.c_str() );
}


// PyMod contract code below
static PyMethodDef GenericIndividualMethods[] =
{
     {"create", create, METH_VARARGS, "Create somebody."},
     {"update", update, METH_VARARGS, "Update somebody."},
     {"get_age", getAge, METH_VARARGS, "Get age."},
     {"is_infected", isInfected, METH_VARARGS, "Has 1+ infections."},
     {"get_immunity", getImmunity, METH_VARARGS, "Returns acquisition immunity (product of immune system and interventions modifier)."},
     {"my_set_callback", my_set_callback, METH_VARARGS, "Set callback."},
     {"get_schema", getSchema, METH_VARARGS, "Update."},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initdtk_genericindividual(void)
{
     (void) Py_InitModule("dtk_genericindividual", GenericIndividualMethods);
}
