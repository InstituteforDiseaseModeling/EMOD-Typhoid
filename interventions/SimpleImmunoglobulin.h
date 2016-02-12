/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Vaccine.h"

namespace Kernel
{
    class SimpleImmunoglobulin : public SimpleVaccine
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, SimpleImmunoglobulin, IDistributableIntervention)

    public:
        SimpleImmunoglobulin();
        bool Configure( const Configuration* pConfig );

    protected:
        DECLARE_SERIALIZABLE(SimpleImmunoglobulin);
    };
}
