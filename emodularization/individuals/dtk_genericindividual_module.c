#include <Python.h>
#include <iostream>

#include "Environment.h"
#include "SimulationConfig.h"
#include "NodeEventContext.h"
#include "Individual.h"
#include "INodeContext.h"
#include "Properties.h"

Kernel::IndividualHuman * person = nullptr;
Configuration * configStubJson = nullptr;

using namespace Kernel;

class StubNode : public INodeContext
{
    public:
        StubNode()
        {
            //IPFactory::CreateFactory();
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
        virtual void VisitIndividuals( INodeEventContext::individual_visit_function_t func) {}
        virtual int VisitIndividuals(IVisitIndividual* pIndividualVisitImpl, int limit = -1) {} 
        virtual const NodeDemographics& GetDemographics() {}
        virtual bool GetUrban() const {}
        virtual IdmDateTime GetTime() const {}
        virtual void UpdateInterventions(float = 0.0f) {} 
        virtual void RegisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, INodeEventContext::TravelEventType type) {}
        virtual void UnregisterTravelDistributionSource(ITravelLinkedDistributionSource *tles, INodeEventContext::TravelEventType type) {} 
        virtual const suids::suid & GetId() const {}
        virtual void SetContextTo(INodeContext* context) {}
        virtual std::list<INodeDistributableIntervention*> GetInterventionsByType(const std::string& type_name) {}
        virtual void PurgeExisting( const std::string& iv_name ) {} 
        virtual bool IsInPolygon(float* vertex_coords, int numcoords) {}
        virtual bool IsInExternalIdSet( const tNodeIdList& nodelist ) {}
        virtual ::RANDOMBASE* GetRng() {}
        virtual INodeContext* GetNodeContext() {} 
        virtual int GetIndividualHumanCount() const {}
        virtual ExternalNodeId_t GetExternalId() const {}


        virtual ISimulationContext* GetParent() override {}
        virtual suids::suid GetSuid() const override {}
        virtual void SetContextTo( ISimulationContext* ) override {}
        virtual void SetMonteCarloParameters(float indsamplerate =.05, int nummininf = 0) override {}
        virtual void SetParameters(NodeDemographicsFactory *demographics_factory, ClimateFactory *climate_factory) override {}
        virtual void PopulateFromDemographics() override {}
        virtual suids::suid GetNextInfectionSuid() override {}
        virtual void Update(float dt) override {}
        virtual IIndividualHuman* processImmigratingIndividual( IIndividualHuman* ) override { return nullptr; }
        virtual void ExposeIndividual(IInfectable* candidate, const TransmissionGroupMembership_t* individual, float dt) override {}
        virtual void DepositFromIndividual(StrainIdentity* strain_IDs, float contagion_quantity, const TransmissionGroupMembership_t* individual) override {}
        virtual void GetGroupMembershipForIndividual(const RouteList_t& route, tProperties* properties, TransmissionGroupMembership_t* membershipOut ) override {}
        virtual void UpdateTransmissionGroupPopulation(const TransmissionGroupMembership_t* membership, float size_changes,float mc_weight) override {}
        virtual float GetTotalContagion(const TransmissionGroupMembership_t* membership) override {}
        virtual const RouteList_t& GetTransmissionRoutes( ) const override {}
        virtual float getSinusoidalCorrection(float sinusoidal_amplitude, float sinusoidal_phase) const override {}
        virtual float getBoxcarCorrection(float boxcar_amplitude, float boxcar_start_time, float boxcar_end_time) const override {}
        virtual act_prob_vec_t DiscreteGetTotalContagion(const TransmissionGroupMembership_t* membership) override {}
        virtual IMigrationInfo* GetMigrationInfo() override {}
        virtual const NodeDemographics* GetDemographics() const override { return nullptr; }
        virtual const NodeDemographicsDistribution* GetDemographicsDistribution(std::string) const override { return nullptr; }
        virtual float       GetInfected()      const override {}
        virtual float       GetStatPop()       const override {}
        virtual float       GetBirths()        const override {}
        virtual float       GetCampaignCost()  const override {}
        virtual float       GetInfectivity()   const override {}
        virtual float       GetInfectionRate() const override {}
        virtual float       GetSusceptDynamicScaling() const override {}
        virtual const Climate* GetLocalWeather() const override {}
        virtual long int GetPossibleMothers()  const override {}
        virtual float GetMeanAgeInfection()    const override {}
        virtual float GetLatitudeDegrees() override {}
        virtual float GetLongitudeDegrees() override {}
        virtual ExternalNodeId_t GetExternalID() const override {}
        virtual INodeEventContext* GetEventContext() override { return nullptr; }
        virtual void AddEventsFromOtherNodes( const std::vector<std::string>& rEventNameList ) override {}
        virtual bool IsEveryoneHome() const override {}
        virtual float GetBasePopulationScaleFactor() const override {}
        virtual ProbabilityNumber GetProbMaternalTransmission() const override {}
        virtual void SetupMigration( IMigrationInfoFactory * migration_factory, MigrationStructure::Enum ms, const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap ) override {}
        virtual std::vector<bool> GetMigrationTypeEnabledFromDemographics() const override {}
        virtual void SetWaitingForFamilyTrip( suids::suid migrationDestination, MigrationType::Enum migrationType, float timeUntilTrip, float timeAtDestination, bool isDestinationNewHome ) override {}
};
StubNode node;

static void initInd( bool dr = false )
{
    if( configStubJson == nullptr )
    {
        configStubJson = Configuration::Load("gi.json");
        std::cout << "configStubJson initialized from gi.json." << std::endl;
        Kernel::JsonConfigurable::_useDefaults = true;
        Kernel::IndividualHuman::InitializeStatics( configStubJson );
        std::cout << "Initialized Statics from gi.json." << std::endl;
        Kernel::JsonConfigurable::_useDefaults = false; 
        person->SetParameters( &node, 0.0f, 0.0f, 0.0f, 0.0f );
    }
}


static PyObject*
create(PyObject* self, PyObject* args)
{
    //const char* name;

    //if (!PyArg_ParseTuple(args, "s", &name))
        //return NULL;

    //printf("Hello %s!\n", name);
    person = Kernel::IndividualHuman::CreateHuman( &node, Kernel::suids::nil_suid(), 1.0f, 365.0f, 0, 0 );
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
