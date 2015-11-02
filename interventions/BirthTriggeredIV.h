/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>

#include "BoostLibWrapper.h"

#include "Interventions.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "NodeEventContext.h"
#include "Configure.h"

namespace Kernel
{
    class BirthTriggeredIV : public IIndividualEventObserver, public BaseNodeIntervention // , public INodeDistributableInterventionParameterSetterInterface
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, BirthTriggeredIV, INodeDistributableIntervention)
    
        friend class CalendarEventCoordinator;
    
    public:
        BirthTriggeredIV();
        virtual ~BirthTriggeredIV();
        virtual int AddRef() override;
        virtual int Release() override;
        /* clorton virtual */ bool Configure( const Configuration* config ) /* clorton override */;

        // INodeDistributableIntervention
        virtual bool Distribute( INodeEventContext *pNodeEventContext, IEventCoordinator2 *pEC ) override;
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;
        virtual void SetContextTo(INodeEventContext *context) override;
        virtual void Update(float dt) override;

        // IIndividualEventObserver
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, const std::string& StateChange ) override;

        virtual void SetDemographicCoverage(float new_coverage) {demographic_coverage = new_coverage;};
        virtual void SetMaxDuration(float new_duration) {max_duration = new_duration;};

    protected:
        INodeEventContext* parent;

        float max_duration;
        float duration;
        float demographic_coverage;

        IndividualInterventionConfig actual_intervention_config;

#if USE_BOOST_SERIALIZATION
    private:
        template<class Archive>
        friend void serialize(Archive &ar, BirthTriggeredIV& iv, const unsigned int v);
#endif
    };
}
