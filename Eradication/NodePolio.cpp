/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include <math.h>
#include <numeric> // for std::accumulate
#include <functional> // why not algorithm?
#include "Sugar.h"
#include "Exceptions.h"
#include "NodePolio.h"
#include "IndividualPolio.h"
#include "SimulationConfig.h"

using namespace Kernel;

static const char* _module = "NodePolio";
namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(NodePolio, Node)
        HANDLE_INTERFACE(INodePolio)
    END_QUERY_INTERFACE_DERIVED(NodePolio, Node)


    NodePolio::NodePolio() : NodeEnvironmental() { }

    NodePolio::NodePolio(ISimulationContext *_parent_sim, suids::suid node_suid) : NodeEnvironmental(_parent_sim, node_suid)
    {
        contagion = 0;
        window_index = 0;
        virus_lastReportTime = -1;

        mean_age_infection = 0;
        n_people_age_infection = 0;
        newInfectedPeople = 0;
        newInfectedPeopleAgeProduct = 0;
    }

    void NodePolio::Initialize()
    {
        NodeEnvironmental::Initialize();
    }

    bool NodePolio::Configure(
        const Configuration* config
    )
    {
        return NodeEnvironmental::Configure( config );
    }

    NodePolio *NodePolio::CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid)
    {
        NodePolio *newnode = _new_ NodePolio(_parent_sim, node_suid);
        newnode->Initialize();

        return newnode;
    }

    float
    NodePolio::GetMeanAgeInfection() const
    {
        return mean_age_infection;
    }
}

Kernel::NodePolio::~NodePolio(void)
{
}

const float Kernel::NodePolio::virus_reporting_interval = 5.0f;


void Kernel::NodePolio::resetNodeStateCounters(void)
{
    Kernel::NodeEnvironmental::resetNodeStateCounters();
    
    newInfectedPeople = 0;
    newInfectedPeopleAgeProduct = 0;

    newDiseaseSusceptibleInfections = 0.0f;
    newDiseaseSusceptibleInfectionsUnder5 = 0.0f;
    newDiseaseSusceptibleInfectionsOver5 = 0.0f;
}

void Kernel::NodePolio::updateNodeStateCounters(IndividualHuman *ih)
{
    float mc_weight                = float(ih->GetMonteCarloWeight());
    IIndividualHumanPolio *tempind2 = NULL;
    if( ih->QueryInterface( GET_IID( IIndividualHumanPolio ), (void**)&tempind2 ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "tempind2", "IndividualHumanPolio", "IndividualHuman" );
    }

    NewInfectionState::_enum tmpNewInfFlag = ih->GetNewInfectionState();

    if( tmpNewInfFlag == NewInfectionState::NewInfection ||
        tmpNewInfFlag == NewInfectionState::NewAndDetected )
    {
        newInfectedPeople += mc_weight;
        newInfectedPeopleAgeProduct += mc_weight * (tempind2->GetAgeOfMostRecentInfection());
    }

    Kernel::NodeEnvironmental::updateNodeStateCounters(ih);

    tempind2->ClearAllNewInfectionByStrain();
}


void Kernel::NodePolio::finalizeNodeStateCounters(void)
{
    Kernel::NodeEnvironmental::finalizeNodeStateCounters();
   
    // rolling average of the age of infection, over number of time steps = infection_averaging_window
    infected_people_prior.push_back( (float)newInfectedPeople );
    if( infected_people_prior.size() > 30 )
    {
        infected_people_prior.pop_front();
    }
    if( newInfectedPeopleAgeProduct < 0 )
    {
        throw CalculatedValueOutOfRangeException( __FILE__, __LINE__, __FUNCTION__, "newInfectedPeopleAgeProduct", newInfectedPeopleAgeProduct, 0 );
    }

    infected_age_people_prior.push_back( (float)newInfectedPeopleAgeProduct );
    if( infected_age_people_prior.size() > 30 )
    {
        infected_age_people_prior.pop_front();
    }

    double numerator = std::accumulate(infected_age_people_prior.begin(), infected_age_people_prior.end(), 0.0);
    if( numerator < 0.0 )
    {
        throw CalculatedValueOutOfRangeException( __FILE__, __LINE__, __FUNCTION__, "numerator", numerator, 0 );
    }

    float denominator = std::accumulate( infected_people_prior.begin(), infected_people_prior.end(), 0 );
    if( denominator && numerator )
    {
        mean_age_infection = numerator/( denominator * DAYSPERYEAR);
        LOG_DEBUG_F( "mean_age_infection = %f/%f*365.\n", numerator, denominator );
    }
    else
    {
        mean_age_infection = 0; // necessary?
    }
}

void Kernel::NodePolio::LoadImmunityDemographicsDistribution()
{
    demographic_distributions[NodeDemographicsDistribution::maternal_antibody_distribution1] = NodeDemographicsDistribution::CreateDistribution(demographics["maternal_antibody_distribution1"]);
    demographic_distributions[NodeDemographicsDistribution::maternal_antibody_distribution2] = NodeDemographicsDistribution::CreateDistribution(demographics["maternal_antibody_distribution2"]);
    demographic_distributions[NodeDemographicsDistribution::maternal_antibody_distribution3] = NodeDemographicsDistribution::CreateDistribution(demographics["maternal_antibody_distribution3"]);
    demographic_distributions[NodeDemographicsDistribution::mucosal_memory_distribution1] = NodeDemographicsDistribution::CreateDistribution(demographics["mucosal_memory_distribution1"],       "age");
    demographic_distributions[NodeDemographicsDistribution::mucosal_memory_distribution2] = NodeDemographicsDistribution::CreateDistribution(demographics["mucosal_memory_distribution2"],       "age");
    demographic_distributions[NodeDemographicsDistribution::mucosal_memory_distribution3] = NodeDemographicsDistribution::CreateDistribution(demographics["mucosal_memory_distribution3"],       "age");
    demographic_distributions[NodeDemographicsDistribution::humoral_memory_distribution1] = NodeDemographicsDistribution::CreateDistribution(demographics["humoral_memory_distribution1"],       "age", "mucosal_memory");
    demographic_distributions[NodeDemographicsDistribution::humoral_memory_distribution2] = NodeDemographicsDistribution::CreateDistribution(demographics["humoral_memory_distribution2"],       "age", "mucosal_memory");
    demographic_distributions[NodeDemographicsDistribution::humoral_memory_distribution3] = NodeDemographicsDistribution::CreateDistribution(demographics["humoral_memory_distribution3"],       "age", "mucosal_memory");
    demographic_distributions[NodeDemographicsDistribution::fake_time_since_last_infection_distribution] = NodeDemographicsDistribution::CreateDistribution(demographics["fake_time_since_infection_distribution"],       "age");
}

float Kernel::NodePolio::drawInitialImmunity(float ind_init_age)
{
    switch( params()->immunity_initialization_distribution_type ) 
    {
    case DistributionType::DISTRIBUTION_COMPLEX:
    case DistributionType::DISTRIBUTION_OFF:
        // For POLIO_SIM, ImmunityDistributionFlag, ImmunityDistribution1, ImmunityDistribution2 are unused
        return 1.0f;

    case DistributionType::DISTRIBUTION_SIMPLE:
        throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Immunity_Initialization_Distribution_Type", "DISTRIBUTION_SIMPLE", "Simulation_Type", "POLIO_SIM");

    default:
        if( !JsonConfigurable::_dryrun )
        {
            throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "Immunity_Initialization_Distribution_Type", params()->immunity_initialization_distribution_type, DistributionType::pairs::lookup_key( params()->immunity_initialization_distribution_type ) );
        }
    }

    return 1.0f;
}

Kernel::IndividualHuman *Kernel::NodePolio::createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty)
{
    return Kernel::IndividualHumanPolio::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender, above_poverty);
}

const 
Kernel::SimulationConfig*
NodePolio::params()
{
    return GET_CONFIGURABLE(SimulationConfig);
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(Kernel::NodePolio)
namespace Kernel {
    template<class Archive>
    void serialize(Archive & ar, NodePolio& node, const unsigned int /* file_version */)
    { 
        ar.template register_type<IndividualHumanPolio>();
        ar & boost::serialization::base_object<NodeEnvironmental>(node);
        ar & node.mean_age_infection;
        ar & node.n_people_age_infection;
        ar & node.newInfectedPeople;
        ar & node.newInfectedPeopleAgeProduct;

        ar & node.window_index;
        ar & node.infected_people_prior;
        ar & node.infected_age_people_prior;
    }
}
#endif

Kernel::NodePolioTest *
NodePolioTest::CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid)
{
    auto *newnode = _new_ NodePolioTest(_parent_sim, node_suid);
    newnode->Initialize();

    return newnode;
}

Kernel::NodePolioTest::NodePolioTest(ISimulationContext *_parent_sim, suids::suid node_suid)
{
    std::cout << "TestNodePolio ctor." << std::endl;
    parent = _parent_sim;
    auto newPerson = configureAndAddNewIndividual(1.0F /*mc*/, 0 /*age*/, 0.0f /*prev*/, 0.5f /*gender*/); // N.B. temp_prevalence=0 without maternal_transmission flag
    for (auto pIndividual : individualHumans)
    {
         // Nothing to do at the moment.
    }
}
#endif // ENABLE_POLIO
