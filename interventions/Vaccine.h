/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Interventions.h"
#include "SimpleTypemapRegistration.h"
#include "Contexts.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "Configure.h"

namespace Kernel
{
    struct IVaccineConsumer;
    struct ICampaignCostObserver;

    struct IVaccine : public ISupports
    {
        virtual void  ApplyVaccineTake()                = 0;
        virtual ~IVaccine() { } // needed for cleanup via interface pointer
    };

    class SimpleVaccine : public BaseIntervention, public IVaccine
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, SimpleVaccine, IDistributableIntervention)

    public:
        SimpleVaccine();
        virtual ~SimpleVaccine();
        virtual int AddRef() { return BaseIntervention::AddRef(); }
        virtual int Release() { return BaseIntervention::Release(); }
        bool Configure( const Configuration* pConfig );

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO );
        virtual void SetContextTo(IIndividualHumanContext *context);
        virtual void Update(float dt);

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);

        // IVaccine
        virtual int   GetVaccineType()            const;    // clorton - still needed?
        virtual void  ApplyVaccineTake();

    protected:
        // context for this intervention--does not need to be reset upon migration, it is just for GiveVaccine()
        IIndividualHumanContext *parent;

    protected:
        int   vaccine_type;
        float vaccine_take;
        float current_reducedacquire;
        float current_reducedtransmit;
        float current_reducedmortality;
        InterventionDurabilityProfile::Enum durability_time_profile;
        float primary_decay_time_constant;
        float secondary_decay_time_constant;
        IVaccineConsumer * ivc; // interventions container

        DECLARE_SERIALIZABLE(SimpleVaccine);
    };
}
