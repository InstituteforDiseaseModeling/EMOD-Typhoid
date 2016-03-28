/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Vaccine.h"
#include "PolioContexts.h"

namespace Kernel
{
    class PolioVaccine : public IVaccine, public BaseIntervention, public IPolioVaccine
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, PolioVaccine, IDistributableIntervention)

    public:
        static PolioVaccine* CreateVaccine(PolioVaccineType::Enum type, float days_since_vaccine);
        PolioVaccine();
        virtual ~PolioVaccine();
        virtual bool Configure( const Configuration* config ) override;
        virtual int AddRef() override { return BaseIntervention::AddRef(); }
        virtual int Release() override { return BaseIntervention::Release(); }

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO ) override;
        virtual void SetContextTo(IIndividualHumanContext *context) override { /* not needed for this intervention */ }
        virtual void Update(float dt) override;

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;

        // IPolioVaccine
        virtual PolioVaccineType::Enum GetVaccineType() const override;

        // IVaccine
        virtual void  ApplyVaccineTake() override {} // Take is handled in live vaccines
        
        /* clorton virtual */ float GetVaccineReducedAcquire()  const { return 0; }
        /* clorton virtual */ float GetVaccineReducedTransmit() const { return 0; }

    protected:
        PolioVaccineType::Enum vaccine_type;
        float time_since_vaccination;
        IVaccineConsumer *ivc;

        DECLARE_SERIALIZABLE(PolioVaccine);
    };
}
