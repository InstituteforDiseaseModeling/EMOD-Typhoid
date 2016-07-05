/*****************************************************************************

Copyright (c) 2012 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once

#include "Drugs.h"
//#include "HIVEnums.h"
#include "HIVInterventionsContainer.h"  // For IHIVIntervention 

namespace Kernel
{
    //------------------------------ Anti-HIV Monotherapy Drug -------------------------------------------
    struct IHIVDrugEffectsApply;

    class AntiHIVMonotherapyDrug : public GenericDrug, public IHIVIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, AntiHIVMonotherapyDrug, IDistributableIntervention);

    public:
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        bool Configure( const Configuration * );
        AntiHIVMonotherapyDrug();
        virtual ~AntiHIVMonotherapyDrug();

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void SetContextTo(IIndividualHumanContext *context);

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO );

        // IDrug
        virtual void  ConfigureDrugTreatment( IIndividualHumanInterventionsContext * ivc = NULL );
        virtual std::string GetDrugName();

    protected:
        // These have same names as analogous methods on container but are internal for the drug itself.
        virtual float GetDrugInactivationRate();
        virtual float GetDrugClearanceRate();

        float get_drug_viral_suppression_efficacy();
        float get_drug_concentration();

        virtual void ApplyEffects();            // DJK: ApplyEffects() --> ApplyAction() <ERAD-1853>

        IHIVDrugEffectsApply * ihivda;

        HIVDrugClass::Enum hiv_drug_class;        // Class of HIV drug
        ReverseTranscriptaseNucleosideAnalog::Enum nucleoside_analog;   // Used only for NRTI drugs to identify target
        std::string drug_name;

        ICampaignCostObserver * m_pCCO;

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        // Serialization
        friend class ::boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive &ar, AntiHIVMonotherapyDrug& drug, const unsigned int v);
#endif
    };
}
