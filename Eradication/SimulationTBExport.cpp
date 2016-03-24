/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "SimulationTB.h"
#include "ReportTB.h"
#include "NodeTB.h"
#include "InfectionTB.h"
#include "SusceptibilityTB.h"
#include "Sugar.h"
#include "SimulationConfig.h"
#include "ProgVersion.h"

#pragma warning(disable : 4996)

static const char * _module = "SimulationTB";
#ifdef _TB_DLL

// Note: _diseaseType has to match the Simulation_Type name in config.json
static std::string _diseaseType = "TB_SIM";

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

//
// This is the interface function from the DTK.
//
DTK_DLLEXPORT Kernel::ISimulation* __cdecl
CreateSimulation(
    const Environment * pEnv 
)
{
    Environment::setInstance(const_cast<Environment*>(pEnv));
    LOG_INFO_F("CreateSimulation called for\n");
    return Kernel::SimulationTB::CreateSimulation( EnvPtr->Config );
}

DTK_DLLEXPORT
const char *
__cdecl
GetDiseaseType()
{
    LOG_INFO("GetDiseaseType called\n");
    return _diseaseType.c_str();
}

DTK_DLLEXPORT
char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    Environment::setInstance(const_cast<Environment*>(pEnv));
    ProgDllVersion pv;
    LOG_INFO_F("GetEModuleVersion called with ver=%s\n", pv.getVersion());
    if (sVer) strcpy(sVer, pv.getVersion());
    return sVer;
}

DTK_DLLEXPORT
const char* __cdecl
GetSchema()
{
    LOG_DEBUG_F("GetSchema called for %s: map has size %d\n", _module, Kernel::JsonConfigurable::get_registration_map().size() );
    
    json::Object configSchemaAll;
    for (auto& entry : Kernel::JsonConfigurable::get_registration_map())
    {
        const std::string classname = entry.first;
        LOG_DEBUG_F( "classname = %s\n", classname.c_str() );
        json::QuickBuilder config_schema = ((*(entry.second))());
        configSchemaAll[classname] = config_schema;
    }

    std::ostringstream schemaJsonString;
    json::Writer::Write( configSchemaAll, schemaJsonString );

    putenv( ( std::string( "GET_SCHEMA_RESULT=" ) + schemaJsonString.str().c_str() ).c_str() );
    return schemaJsonString.str().c_str();
}

#ifdef __cplusplus
}
#endif

#endif

