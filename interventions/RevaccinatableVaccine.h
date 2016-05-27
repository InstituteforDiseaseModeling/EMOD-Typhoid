/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Vaccine.h"
#include "EventTrigger.h"

namespace Kernel
{
    struct IRevaccinatableVaccine : ISupports
    {
        virtual bool AllowRevaccination() const = 0;
    };

    class RevaccinatableVaccine : public SimpleVaccine, public IRevaccinatableVaccine
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, RevaccinatableVaccine, IDistributableIntervention)
        DECLARE_QUERY_INTERFACE()
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        RevaccinatableVaccine();
        RevaccinatableVaccine( const RevaccinatableVaccine& );
        virtual ~RevaccinatableVaccine();


        // SimpleVaccine
        virtual bool Configure( const Configuration* pConfig ) override;
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO ) override;
        virtual void Update(float dt) override;

    protected:
        virtual bool AllowRevaccination() const override;

        float        m_DurationToWaitBeforeRevaccination;
        float        m_TimeSinceVaccination;
        EventTrigger m_DistributedEventTrigger;
        EventTrigger m_ExpiredEventTrigger;

        DECLARE_SERIALIZABLE(RevaccinatableVaccine);
    };
}
