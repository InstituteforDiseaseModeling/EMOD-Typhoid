/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/
#pragma once

#ifdef ENABLE_POLIO
#include <string>
#include <list>
#include <vector>

#include "Drugs.h"
#include "Interventions.h"
#include "InterventionsContainer.h"
#include "PolioContexts.h"

namespace Kernel
{
    class PolioInterventionsContainer : public InterventionsContainer,
                                        public IPolioVaccineEffects,
                                        public IPolioDrugEffects,
                                        IPolioDrugEffectsApply
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        PolioInterventionsContainer();
        virtual ~PolioInterventionsContainer();

        virtual QueryResult QueryInterface(iid_t iid, void** pinstance) override;

        // IVaccineConsumer: not any more!
        virtual bool GiveIntervention( IDistributableIntervention * pIV ) override;

        // IPolioVaccineEffects
        virtual std::list<IPolioVaccine*>& GetNewVaccines() override;
        virtual void ClearNewVaccines() override;

        // IPolioDrugEffectsApply
        virtual void ApplyDrugTiterEffect( float rate ) override;
        virtual void ApplyDrugDurationEffect( float rate ) override;
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ) override; // not used for anything
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ) override; // not used for anything

        //IPolioDrugEffects(Get)
        virtual float get_titer_efficacy() const override;
        virtual float get_infection_duration_efficacy() const override;

        virtual void Update(float dt) override; // example of intervention timestep update

    protected:
        std::list<IPolioVaccine*> new_vaccines;
        void GiveDrug(IDrug* drug);
        float titer_efficacy;
        float infection_duration_efficacy;

        DECLARE_SERIALIZABLE(PolioInterventionsContainer);
    };
}

#endif