/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "FactorySupport.h"
#include "InterventionFactory.h"
#include "MosquitoRelease.h"
#include "ProgVersion.h"

static const char * _module = "MosquitoRelease";

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
    std::cout << "RegisterWithFactory called for " << _module << std::endl;
    pInterventionFactory->Register(
        _module,
        []()
        {
            return (Kernel::ISupports*)(Kernel::INodeDistributableIntervention*)(_new_ Kernel::MosquitoRelease());
        }
    );
    return true;

}

#ifdef __cplusplus
}
#endif
