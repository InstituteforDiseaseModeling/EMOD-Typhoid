/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#if defined(ENABLE_POLIO) || defined(ENABLE_TYPHOID)

#include "Sugar.h"
#include "InfectionEnvironmental.h"

static const char* _module = "InfectionEnvironmental";

namespace Kernel
{
    InfectionEnvironmental::InfectionEnvironmental()
    {
    }

    InfectionEnvironmental::~InfectionEnvironmental(void)
    { }

    void InfectionEnvironmental::Update(float dt, ISusceptibilityContext* immunity)
    {
        Infection::Update(dt);
        //for Environmental, same infectiousness is deposited to both environmental and contact routes (if applicable)
        //By default (if HINT is off), only environmental route is available.
        infectiousnessByRoute[string("environmental")] = infectiousness;
        infectiousnessByRoute[string("contact")] = infectiousness;
    }

    void InfectionEnvironmental::SetParameters(StrainIdentity* _infstrain, int incubation_period_override )
    {
        Infection::SetParameters(_infstrain,incubation_period_override);

        // Infection::SetParameters() used to leave infectiousness == 0.0f in the
        // case where incubation_timer <= 0. This didn't allow for strict SI[R]
        // models so it changed to set infectiousness = base_infectivity in this
        // case. We didn't want to change the behavior of environmental/polio, so we
        // undo that here.
        if (incubation_timer <= 0)
        {
            infectiousness = 0.0f;
        }
        if (incubation_timer <= 0)
        {
            infectiousness = 0.0f;
        }

        infectiousnessByRoute[string("environmental")]=0.0f;
        infectiousnessByRoute[string("contact")]=0.0f;
    }

    InfectionEnvironmental::InfectionEnvironmental(IIndividualHumanContext *context) : Kernel::Infection(context)
    {
    }

    void InfectionEnvironmental::Initialize(suids::suid _suid)
    {
        Kernel::Infection::Initialize(_suid);
    }

    InfectionEnvironmental *InfectionEnvironmental::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        InfectionEnvironmental *newinfection = _new_ InfectionEnvironmental(context);
        newinfection->Initialize(_suid);

        return newinfection;
    }

    REGISTER_SERIALIZABLE(InfectionEnvironmental);

    void InfectionEnvironmental::serialize(IArchive& ar, InfectionEnvironmental* obj)
    {
        Infection::serialize(ar, obj);
        // nothing else, yet
    }
}

#endif // ENABLE_POLIO
