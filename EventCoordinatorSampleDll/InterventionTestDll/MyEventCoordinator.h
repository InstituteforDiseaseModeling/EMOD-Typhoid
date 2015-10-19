/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>
#include <map>

#include "EventCoordinator.h"
#include "Configure.h"
#include "NodeEventContext.h"
#include "Interventions.h" 

namespace Kernel
{
    //////////////////////////////////////////////////////////////////////////
    // Example implementations

    // Standard distribution ec that just gives out the intervention once to the fraction of people specified by the coverage parameter 
    class MyEventCoordinator : public IEventCoordinator, public ITravelLinkedDistributionSource, public IVisitIndividual, public IEventCoordinator2, public JsonConfigurable
    {
#ifdef WIN32
        DECLARE_SERIALIZABLE(MyEventCoordinator)
#endif
        DECLARE_FACTORY_REGISTERED(EventCoordinatorFactory, MyEventCoordinator, IEventCoordinator)    
        DECLARE_CONFIGURED(MyEventCoordinator)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    public:
        MyEventCoordinator();

        // IEventCoordinator
        virtual void SetContextTo(ISimulationEventContext *isec);
        
        virtual void AddNode( const suids::suid& node_suid);

        virtual void Update(float dt);
        //virtual void UpdateNode(INodeDistributionContext *ndc) = 0;
        virtual void UpdateNodes(float dt);
        virtual bool visitIndividualCallback(IIndividualHumanEventContext *ihec, float &incrementalCostOut, ICampaignCostObserver * pICCO );

        virtual bool IsFinished(); // returns false when the EC requires no further updates and can be disposed of

        // ITravelLinkedDistributionSource
        virtual void ProcessDeparting(IIndividualHumanEventContext *dc);
        virtual void ProcessArriving(IIndividualHumanEventContext *dc);

        virtual int GetNumDistributionsPerNode() const;

        static IInterventionFactory * _interventionFactory; // encapsulate!

    protected:
        ISimulationEventContext  *parent;

        // attributes      
        
        bool  distribution_complete;
        int   num_repetitions;
        int   days_between_reps;
        float demographic_coverage;
        TargetDemographicType::Enum target_demographic;
        float cost_to_consumer;
        float target_age_min;
        float target_age_max;
        int   num_distributions;
        int   include_emigrants;
        int   include_immigrants;
        
        Configuration *intervention_config;
        std::vector<INodeEventContext*> cached_nodes;
        std::vector<suids::suid> node_suids; // to help with serialization


        // helpers

        void regenerateCachedNodeContextPointers();

    private:
        int days_since_last;
        int intervention_activated;

        ///////////////////////////////////////////////////////////////////////////
        // Serialization
        friend class ::boost::serialization::access;

        template<class Archive>
        void serialize_inner(Archive &ar, const unsigned int v)
        {
            ar & distribution_complete;
            ar & num_repetitions;
            ar & days_between_reps;
            ar & demographic_coverage;
            ar & target_demographic;
            ar & target_age_min;
            ar & target_age_max;
            ar & num_distributions;
            ar & include_emigrants;
            ar & include_immigrants;
            ar & intervention_config;
            
            // need to save the list of suids and restore from them, rather than saving the context pointers
            //ar & cached_nodes;
            ar & node_suids;                
     
        }

        bool qualifiesDemographically( const IIndividualHumanEventContext * pIndividual ) const;
    };
}
