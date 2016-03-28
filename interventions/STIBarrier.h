/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "BoostLibWrapper.h"

#include "Interventions.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "FactorySupport.h"
#include "Configure.h"
#include "IRelationship.h"

namespace Kernel
{
    struct ISTIBarrierConsumer; 

    /* Keep around as an identity solution??? */
    struct ISTIBarrier : public ISupports
    {
    };

    class STIBarrier : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, STIBarrier, IDistributableIntervention)

    public:
        STIBarrier();
        virtual ~STIBarrier() { }

        bool Configure( const Configuration * config );

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO );
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void SetContextTo(IIndividualHumanContext *context);
        virtual void Update(float dt);

    protected:
        float early;
        float late;
        float midyear;
        float rate;
        RelationshipType::Enum rel_type;
        ISTIBarrierConsumer *ibc;

        DECLARE_SERIALIZABLE(STIBarrier);
    };
}
