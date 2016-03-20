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
        virtual ~SimpleImmunoglobulin();
        SimpleImmunoglobulin( const SimpleImmunoglobulin& );
        bool Configure( const Configuration* pConfig );
        virtual void Update(float dt) override;

    protected:
        DECLARE_SERIALIZABLE(SimpleImmunoglobulin);
	// TBD: Will probably move this down to vaccine but for now want it here until I figure out details
        WaningConfig   acquire_config;
        IWaningEffect* acquire_effect;
        WaningConfig   transmit_config;
        IWaningEffect* transmit_effect;
    };
}
