/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "stdafx.h"

#include "Contexts.h"
#include "Individual.h"

namespace Kernel
{
    class IndividualHumanEnvironmental : public IndividualHuman
    {
    public:    
        DECLARE_QUERY_INTERFACE()

        static IndividualHumanEnvironmental *CreateHuman(INodeContext *context, suids::suid _suid, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual ~IndividualHumanEnvironmental(void);

        virtual void CreateSusceptibility(float = 1.0, float = 1.0) override;

        virtual void ExposeToInfectivity(float dt = 1.0, const TransmissionGroupMembership_t* transmissionGroupMembership = nullptr) override;
        virtual void UpdateInfectiousness(float dt) override;

    protected:
        IndividualHumanEnvironmental( suids::suid id = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);

        // Factory methods
        virtual IInfection* createInfection(suids::suid _suid) override;
        virtual void ReportInfectionState() override;

        DECLARE_SERIALIZABLE(IndividualHumanEnvironmental);
    };
}
