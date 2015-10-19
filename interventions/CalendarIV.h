/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "BoostLibWrapper.h"

#include "Interventions.h"
#include "SimpleTypemapRegistration.h"
#include "Contexts.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "Configure.h"
#include "HealthSeekingBehavior.h"

namespace Kernel
{
    class TargetAgeArrayConfig : public JsonConfigurable
    {
        friend class ::boost::serialization::access;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) { return e_NOINTERFACE; }

        public:
            TargetAgeArrayConfig() {}
            virtual void ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key );
            virtual json::QuickBuilder GetSchema();
            std::map< float, float > age2ProbabilityMap;
            bool dropout;
    };

    class IVCalendar : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, IVCalendar, IDistributableIntervention)

    public:
        // We inherit AddRef/Release abstractly through IHealthSeekBehavior,
        // even though BaseIntervention has a non-abstract version.
        virtual int32_t AddRef() { return BaseIntervention::AddRef(); }
        virtual int32_t Release() { return BaseIntervention::Release(); }

        IVCalendar();
        virtual ~IVCalendar();
        bool Configure( const Configuration* config );

        // IDistributingDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pICCO );
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void SetContextTo(IIndividualHumanContext *context) { parent = context; } // for rng
        virtual void Update(float dt);

    protected:
        IIndividualHumanContext *parent;
        TargetAgeArrayConfig target_age_array; // json list of floats
        IndividualInterventionConfig actual_intervention_config;
        bool dropout;

    private:
        std::list<float> scheduleAges;
        std::string dumpCalendar();
        
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class ::boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive &ar, IVCalendar& cal, const unsigned int v);
#endif        
    };
}
