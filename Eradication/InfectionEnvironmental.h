/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Contexts.h"
#include "Infection.h"

namespace Kernel
{
    class InfectionEnvironmental : public Infection
    {
    public:
        static InfectionEnvironmental *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~InfectionEnvironmental(void);

        virtual void Update(float dt, ISusceptibilityContext* immunity = nullptr) override;
        virtual void SetParameters(StrainIdentity* _infstrain=nullptr, int incubation_period_override = -1 ) override;

    protected:
        InfectionEnvironmental(IIndividualHumanContext *context);
        /* clorton virtual */ void Initialize(suids::suid _suid) /* clorton override */;
        InfectionEnvironmental();

        DECLARE_SERIALIZABLE(InfectionEnvironmental);
    };
}
