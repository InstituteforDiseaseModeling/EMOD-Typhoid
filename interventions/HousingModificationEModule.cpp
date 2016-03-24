/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "FactorySupport.h"
#include "InterventionFactory.h"
#include "HousingModification.h"
#include "ProgVersion.h"

#pragma warning(disable : 4996)

static const char * _module = "IRSHousingModification";
//static const char * _module = "SimpleHousingModification";

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

//
// This is the interface function from the DTK.
//
DTK_DLLEXPORT
char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    Environment::setInstance(const_cast<Environment*>(pEnv));
    ProgDllVersion pv;
    LOG_INFO_F("GetVersion called with ver=%s\n", pv.getVersion());
    if (sVer) strcpy(sVer, pv.getVersion());
    return sVer;
}

DTK_DLLEXPORT int __cdecl
RegisterWithFactory(
    Kernel::IInterventionFactory * pInterventionFactory
)
{
    LOG_DEBUG_F("RegisterWithFactory called for %s\n", _module);
    pInterventionFactory->Register(
        "SimpleHousingModification",
        []()
        {
            return (Kernel::ISupports*)(Kernel::INodeDistributableIntervention*)(_new_ Kernel::SimpleHousingModification());
        }
    );
    pInterventionFactory->Register(
        "IRSHousingModification",
        []()
        {
            return (Kernel::ISupports*)(Kernel::INodeDistributableIntervention*)(_new_ Kernel::IRSHousingModification());
        }
    );
    pInterventionFactory->Register(
        "ScreeningHousingModification",
        []()
        {
            return (Kernel::ISupports*)(Kernel::INodeDistributableIntervention*)(_new_ Kernel::ScreeningHousingModification());
        }
    );
    pInterventionFactory->Register(
        "SpatialRepellentHousingModification",
        []()
        {
            return (Kernel::ISupports*)(Kernel::INodeDistributableIntervention*)(_new_ Kernel::SpatialRepellentHousingModification());
        }
    );
    pInterventionFactory->Register(
        "ArtificialDietHousingModification",
        []()
        {
            return (Kernel::ISupports*)(Kernel::INodeDistributableIntervention*)(_new_ Kernel::ArtificialDietHousingModification());
        }
    );
    pInterventionFactory->Register(
        "InsectKillingFenceHousingModification",
        []()
        {
            return (Kernel::ISupports*)(Kernel::INodeDistributableIntervention*)(_new_ Kernel::InsectKillingFenceHousingModification());
        }
    );

    return true;
}

#ifdef __cplusplus
}
#endif

//SimpleHousingModification
