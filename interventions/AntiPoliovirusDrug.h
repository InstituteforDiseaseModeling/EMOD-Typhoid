/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Drugs.h"

namespace Kernel
{
    struct IPolioDrugEffectsApply;
    class AntipoliovirusDrug : public GenericDrug
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, AntipoliovirusDrug, IDistributableIntervention)

    public:
        bool Configure( const Configuration * );
        AntipoliovirusDrug();
        virtual ~AntipoliovirusDrug();

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void SetContextTo(IIndividualHumanContext *context);

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO );

        // IDrug
        virtual void  ConfigureDrugTreatment();  // read in PkPd and drug-killing parameters from Susceptibility Flags

    protected:

        // These have same names as analogous methods on container but are internal for the drug itself.

        float adherence_rate;
        float titer_efficacy;
        float infection_duration_efficacy;
        float response_probability;

        virtual void ApplyEffects();
        virtual void ResetForNextDose(float dt);
        IPolioDrugEffectsApply * ipda;
    };
}
