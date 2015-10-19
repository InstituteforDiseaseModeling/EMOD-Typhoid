/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <Windows.h>

//#include "stdafx.h"
typedef int int32_t;
#include <boost/foreach.hpp>
#include <boost/serialization/export.hpp>
#include "Sugar.h"
#include "InterventionFactory.h"
#include "Environment.h"
#include "MyEventCoordinator.h"
#include "Configuration.h"
#include "ConfigurationImpl.h"
#include "FactorySupport.h"

//BOOST_CLASS_EXPORT(Kernel::MyEventCoordinator);


// Let's have some fun and customize this. Log out which nodes get the intervention, and which inviduals, track
// them in a map, make bednet's conditional on some node info we get, and some individual info (not just coverage/
// randomness, then lets' repeat in 30 days 1 time.
// Then test out taking bednets away from migrators (just as a test).
namespace Kernel 
{
    IInterventionFactory * MyEventCoordinator::_interventionFactory = NULL;
#ifdef WIN32
    IMPLEMENT_SERIALIZABLE(MyEventCoordinator)
#endif
    IMPL_QUERY_INTERFACE2(MyEventCoordinator, IEventCoordinator, IConfigurable)

    IMPLEMENT_FACTORY_REGISTERED(MyEventCoordinator)

    REGISTER_SERIALIZATION_VOID_CAST(MyEventCoordinator, IEventCoordinator)
    REGISTER_SERIALIZATION_VOID_CAST(MyEventCoordinator, ITravelLinkedDistributionSource)

    QuickBuilder
    MyEventCoordinator::GetSchema()
    {
        return QuickBuilder ( jsonSchemaBase );
    }

    // ctor
    MyEventCoordinator::MyEventCoordinator()
    : parent(NULL)
    , intervention_config(NULL)
    , distribution_complete(false)
    , num_repetitions(-1)
    , days_between_reps(-1)
    , intervention_activated(false)
    , days_since_last(0)
    , target_demographic(TargetDemographicType::Everyone)
    , demographic_coverage(0)
    {
        log_info << "MyEventCoordinator ctor" << std::endl;
        initConfigTypeMap( "intervention_config", &intervention_config, "The nested json of the actual intervention to be distributed by this event coordinator." );
        initConfigTypeMap( "num_distributions", &num_distributions, "Expected number of interventions to distribute", 0, -1, 1000 );
        initConfigTypeMap( "include_departures", &include_emigrants, "include_departures (1=include node emigrants, 0=leave them alone.)", 0, 1, 0 );
        initConfigTypeMap( "include_arrivals", &include_immigrants, "include_departures (1=include node immigrants, 0=leave them alone.)", 0, 1, 0 );
        initConfigTypeMap( "intervention_config", &intervention_config, "The nested json of the actual intervention to be distributed by this event coordinator." );
        initConfigTypeMap( "num_repetitions", &num_repetitions, "Number of times intervention given, used with days_between_reps.", -1, 1000, -1 );
        initConfigTypeMap( "days_between_reps", &days_between_reps, "Repetition interval.", -1, 10000 /*undefined*/, -1 /*off*/ );
        initConfigTypeMap( "demographic_coverage", &demographic_coverage, "Fraction of individuals in the target demographic that will receive this intervention.");
    }

    bool
    MyEventCoordinator::Configure(
        const Configuration * inputJson
    )
    {
        log_info << "MyEventCoordinator::Configure" << std::endl;
        initConfig( "target_demographic", target_demographic, inputJson, MetadataDescriptor::Enum("target_demographic", "The demographic group (from a list of possibles) targeted by this intervention (e.g., Infants)", MDD_ENUM_ARGS(TargetDemographicType)));
        if ( target_demographic == TargetDemographicType::ExplicitAgeRanges )
        {
            initConfigTypeMap( "target_age_min", &target_age_min, "Lower end of age targeted for intervention, in years.");
            initConfigTypeMap( "target_age_max", &target_age_max, "Upper end of age targeted for intervention, in years.");
        }

        return JsonConfigurable::Configure( inputJson );
    }

    void
    MyEventCoordinator::SetContextTo(
        ISimulationEventContext *isec
    )
    {
        log_info << "MyEventCoordinator set context" << std::endl;
        parent = isec;
        regenerateCachedNodeContextPointers();
    }

    // AddNode
    // EventCoordinators track nodes. Nodes can be used to get individuals, who can be queried for an intervention
    void
    MyEventCoordinator::AddNode(
        const suids::suid& node_suid
    )
    {
        //std::cout << "[DEBUG] target_demographic = " << target_demographic << ", demographic_coverage = " << demographic_coverage << std::endl;

        if( !intervention_activated )
        {
            intervention_activated = true;
            days_since_last = days_between_reps -1; // -1 is hack because Update is called before UpdateNodes and we inc in Update and check in UpdateNodes
        }
        // Store uids and node (event context) pointers
        // Random silliness here just to show we can add logic to condition distribution on node data
        //if( node_suid.data % 2 == 0 )
        {
            node_suids.push_back(node_suid);
            cached_nodes.push_back(parent->GetNodeEventContext(node_suid));
        }
        /*else
        {
            std::cerr << "[DEBUG] Ignored node " << node_suid.data << " in AddNode because odd numbered." << std::endl;
        }*/
    }

    void MyEventCoordinator::Update( float dt )
    {
        log_info << "MyEventCoordinator::Update" << std::endl;
        // Check if it's time for another distribution
        if( intervention_activated && num_repetitions)
        {
            days_since_last++;
        }
    }

    void MyEventCoordinator::UpdateNodes( float dt )
    {
        log_info << "UpdateNodes" << std::endl;
        // Only call VisitNodes on first call and if countdown == 0
        if( days_since_last != days_between_reps )
        {
            return;
        }
        int grandTotal = 0;
        int limitPerNode = -1;
        // Only visit individuals if this is NOT an NTI. Check...
        // Check to see if intervention is an INodeDistributable...
        INodeDistributableIntervention *ndi = _interventionFactory->CreateNDIIntervention(intervention_config);
        INodeDistributableIntervention *ndi2 = NULL;

        std::cerr << "[UpdateNodes] limitPerNode = " << limitPerNode << std::endl;                
        // foreach node...
        foreach(INodeEventContext *nec, cached_nodes)
        {
            if( ndi )
            {
                ndi2 = _interventionFactory->CreateNDIIntervention(intervention_config);
                if(ndi2)
                {
                    assert( ndi2 );
                    if (!ndi2->Distribute( nec, this ) )
                    {
                        ndi2->Release(); // a bit wasteful for now, could cache it for the next fellow
                    }
                    std::cerr << "[DEBUG] Back from Distribute." << std::endl;
                }
            }
            else
            {
                // for ITI, if num_distributions != -1, limit totalIndivGivenIntervention to num_distributions.
                if( num_distributions != -1 && grandTotal >= num_distributions )
                {
                    break;
                }

                // For now, distribute evenly across nodes. 
                assert( nec );
                int totalIndivGivenIntervention = nec->VisitIndividuals( this, limitPerNode );
                grandTotal += totalIndivGivenIntervention;
                std::cout << "[UpdateNodes] gave out " << totalIndivGivenIntervention << " interventions at node " << nec->GetId().data << std::endl;                
            }
        }
        if(ndi)
        {
            assert( ndi );
            ndi->Release();
        }
        days_since_last = 0;
        num_repetitions--;
        if( num_repetitions == 0 )
        {
            distribution_complete = true; // we're done, signal disposal ok
        }

        //distribution_complete = true; // we're done, signal disposal ok
        // this signals each process individually that its ok to clean up, in general if the completion times might be different on different nodes 
        // we'd want to coordinate the cleanup signal in Update()
    }

    bool
    MyEventCoordinator::visitIndividualCallback( 
        IIndividualHumanEventContext *ihec,
        float & incrementalCostOut,
        ICampaignCostObserver * pICCO
    )
    {
        //std::cout << "visitIndividualCallback" << std::endl;
        {
            // Add some arbitrary check on individual to determine if they get a bednet.
            // TODO: Demographic targeting goes here.
            // Add real checks on demographics based on intervention demographic targetting. 
            // Return immediately if we hit a non-distribute condition
            if( target_demographic != TargetDemographicType::Everyone ) // don't waste any more time with checks if we're giving to everyone
            {
                if( qualifiesDemographically( ihec ) == false )
                {
                    //std::cout << "Individual not given intervention because not in target demographic." << std::endl;
                    return false;
                }
            }
            //std::cout << "DEBUG: Individual meets demographic targeting criteria." << std::endl;
#if TESTTEST
            if( ihec->GetAge() > 10 )
            {
                // Not giving bednet because wrong age!
                return false;
            }
#endif
            assert(parent);
            assert(randgen);
            double randomDraw = randgen->e();
            //std::cout << "DEBUG: randomDraw = " << randomDraw << ", demographic_coverage = " << demographic_coverage << std::endl;
            // intervention logic goes here
            if (randomDraw > demographic_coverage)
            {
                incrementalCostOut = 0;
                return false;
            }
            else
            {
                incrementalCostOut = 0;

                std::cout << "DEBUG: We won the random draw." << std::endl;
                assert(ihec);
                // OK, now get the individual's social network/mixing pool/facebook friends...
                const IPoolReadOnly * pPools = ihec->GetSocialNetwork();
                if( pPools )
                {
                    const std::vector<float>& weights = pPools->GetWeights();
                    const std::vector<IPool*>& pools = pPools->GetPools();
                    // Prob giving iv to indiv = (w0*a0 + w1*a1)/(w0+w1) where wi = weighting for pool i and ai = accessibility
                    float denom = 0.0f;
                    float numer = 0.0f;
                    for( int idx=0; idx<pools.size(); idx++ )
                    {
                        float access = pools[idx]->GetAccessibility();
                        denom += weights[idx];
                        numer += weights[idx] * access;
                    }
                    float prob = numer/denom;
                    if (randgen->e() > prob)
                    {
                        std::cout << "[VERBOSE] Mixing pool not accessible." << std::endl;
                        return false;
                    }
                }

                // instantiate and distribute intervention
                // std::cout << "Attempting to instantiate intervention of class " << std::string( (*intervention_config)["class"].As<json::String>() ) << std::endl;
                IDistributableIntervention *di = _interventionFactory->CreateIntervention(intervention_config);
                if (di)
                {
                    // std::cout << "Calling Distribute on regular intervention." << std::endl;
                    // The intervention->Distribute( pHuman )
                    if (!di->Distribute(ihec->GetInterventionsContext(), pICCO ))
                    {
                        di->Release(); // a bit wasteful for now, could cache it for the next fellow
                    }
                    //std::cerr << "[VERBOSE] We distributed an intervention " << di << " to an individual " << ihec << " in node." << " at a cost of " << incrementalCostOut << std::endl;
                }
            }
        }
        return true;
    }

    void MyEventCoordinator::regenerateCachedNodeContextPointers()
    {
        // regenerate the cached INodeEventContext* pointers fromthe cached node suids
        // the fact that this needs to happen is probably a good argument for the EC to own the NodeSet, since it needs to query the SEC for the node ids and context pointers anyway
        cached_nodes.clear();
        foreach (suids::suid node_id, node_suids)
        {
            cached_nodes.push_back(parent->GetNodeEventContext(node_id));
        }
    }

    bool 
    MyEventCoordinator::IsFinished()
    {
        return distribution_complete;
    }

    // private/protected
    bool
    MyEventCoordinator::qualifiesDemographically(
        const IIndividualHumanEventContext * const pIndividual
    )
    const
    {
        bool retQualifies = true;

        if (target_demographic == TargetDemographicType::PossibleMothers &&
                 pIndividual->IsPossibleMother() == 0 )
        {
            //std::cerr << "Individual not given intervention because not possible mother." << std::endl;
            return false;
        }
        else if( target_demographic == TargetDemographicType::ExplicitAgeRanges )
        {
            if( pIndividual->GetAge() < target_age_min * DAYSPERYEAR )
            {
                std::cout << " [VERBOSE] Individual not given intervention because too young (age=" << pIndividual->GetAge() << ") for intervention min age (" << target_age_min << ")." << std::endl;
                return false;
            }
            else if( pIndividual->GetAge() > target_age_max * DAYSPERYEAR )
            {
                std::cout << " [VERBOSE] Individual not given intervention because too old (age=" << pIndividual->GetAge() << ") for intervention max age (" << target_age_max << ")." << std::endl;
                return false;
            }
        }

        return retQualifies;
    }

    // IEventCoordinator2 methods
    int MyEventCoordinator::GetNumDistributionsPerNode() const
    {
        return num_distributions;
    }

    void MyEventCoordinator::ProcessDeparting(
        IIndividualHumanEventContext *pInd
    )
    {
        // log_info << "Individual arriving at node receiving intervention. TODO: enforce demographic and other qualifiers." << std::endl;
        float incrementalCostOut = 0.0f;
        visitIndividualCallback( pInd, incrementalCostOut, NULL /* campaign cost observer */ );
    } // these do nothing for now

    void
    MyEventCoordinator::ProcessArriving(
        IIndividualHumanEventContext *pInd
    )
    {
        // log_info << "Individual arriving at node receiving intervention. TODO: enforce demographic and other qualifiers." << std::endl;
        float incrementalCostOut = 0.0f;
        visitIndividualCallback( pInd, incrementalCostOut, NULL /* campaign cost observer */ );
    }

}

#define EOF (-1)

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

//
// This is the interface function from the DTK.
//
DTK_DLLEXPORT int __cdecl
RegisterWithFactory(
    Kernel::IEventCoordinatorFactory * pFactory,
    Kernel::IInterventionFactory * pInterventionFactory
)
{
    std::cout << "RegisterWithFactory called!!! pFactory = " << pFactory << std::endl;
    //pFactory->Register( std::string( "MyEventCoordinator" ),
    pFactory->Register( "MyEventCoordinator",
        []() { return (Kernel::ISupports*)(Kernel::IEventCoordinator*)(_new_ Kernel::MyEventCoordinator()); } );
    std::cout << "Returning from Factory::Register called!!! " << std::endl;
    Kernel::MyEventCoordinator::_interventionFactory = pInterventionFactory;
    std::cout << "_interventionFactory = " << pInterventionFactory << std::endl;
    return 1;
}

#ifdef __cplusplus
}
#endif

BOOL WINAPI DllMain(
  __in  HINSTANCE hinstDLL,
  __in  DWORD fdwReason,
  __in  LPVOID lpvReserved
)
{
    std::cerr << "MyEventCoordinator DllMain called!" << std::endl;
    return true;
}
