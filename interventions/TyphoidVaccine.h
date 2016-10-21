/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "TyphoidInterventionsContainer.h"
//#include "Interventions.h"

namespace Kernel
{ 
    class TyphoidVaccine : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, TyphoidVaccine, IDistributableIntervention)

    public:
        TyphoidVaccine();
        TyphoidVaccine( const TyphoidVaccine& );
        virtual ~TyphoidVaccine();
        virtual bool Configure( const Configuration* pConfig ) override;
        virtual int AddRef() override { return BaseIntervention::AddRef(); }
        virtual int Release() override { return BaseIntervention::Release(); }
        
        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO ) override;
        virtual void SetContextTo(IIndividualHumanContext *context) override;

        // IDistributableIntervention
        virtual void Update(float dt) override;

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) override;

        TyphoidVaccineMode::Enum vaccine_mode;
        float effect;
        ITyphoidVaccineEffectsApply * itvc; // interventions container
        IIndividualHumanContext *parent;

    protected:
        // context for this intervention--does not need to be reset upon migration, it is just for GiveVaccine()

        DECLARE_SERIALIZABLE(TyphoidVaccine);
    };
}
