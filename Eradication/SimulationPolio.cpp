/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "SimulationPolio.h"
#include "NodePolio.h"
#include "IndividualPolio.h"
#include "InfectionPolio.h"
#include "SusceptibilityPolio.h"
#include "suids.hpp"
#include "ReportPolio.h"
#include "BinnedReportPolio.h"
#include "SpatialReportPolio.h"
#include "ProgVersion.h"

#pragma warning(disable : 4996)

static const char * _module = "SimulationPolio";

#ifdef _POLIO_DLL

// Note: _diseaseType has to match the Simulation_Type name in config.json
static std::string _diseaseType = "POLIO_SIM";

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
    LOG_INFO("CreateSimulation called for \n");
    return Kernel::SimulationPolio::CreateSimulation( EnvPtr->Config );
}

DTK_DLLEXPORT
const char *
__cdecl
GetDiseaseType()
{
    LOG_INFO("GetDiseaseType called for \n");
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
    std::ostringstream schemaJsonString;
    for (auto& entry : Kernel::JsonConfigurable::get_registration_map())
    {
        const std::string classname = entry.first;
        LOG_DEBUG_F( "classname = %s\n", classname.c_str() );
        json::QuickBuilder config_schema = ((*(entry.second))());
        configSchemaAll[classname] = config_schema;
    }

    json::Writer::Write( configSchemaAll, schemaJsonString );

    putenv( ( std::string( "GET_SCHEMA_RESULT=" ) + schemaJsonString.str().c_str() ).c_str() );
    return schemaJsonString.str().c_str();
}
#ifdef __cplusplus
}
#endif

#endif

namespace Kernel
{
    SimulationPolio::SimulationPolio() : SimulationEnvironmental()
    {
        reportClassCreator = ReportPolio::CreateReport;
        binnedReportClassCreator = BinnedReportPolio::CreateReport;
        spatialReportClassCreator = SpatialReportPolio::CreateReport;
    }

    void SimulationPolio::Initialize()
    {
        SimulationEnvironmental::Initialize();
    }

    void SimulationPolio::Initialize(const ::Configuration *config)
    {
        SimulationEnvironmental::Initialize(config);
        IndividualHumanPolio fakeHuman;
        LOG_INFO( "Calling Configure on fakeHuman\n" );
        fakeHuman.Configure( config );
    }

    SimulationPolio *SimulationPolio::CreateSimulation()
    {
        SimulationPolio *newsimulation = _new_ SimulationPolio();
        newsimulation->Initialize();

        return newsimulation;
    }

    SimulationPolio *SimulationPolio::CreateSimulation(const ::Configuration *config)
    {
        SimulationPolio *newsimulation = nullptr;

       
       newsimulation = _new_ SimulationPolio();
       if (newsimulation)
       {
            // This sequence is important: first
            // Creation-->Initialization-->Validation
            newsimulation->Initialize(config);
            if(!ValidateConfiguration(config))
            {
                delete newsimulation;
                newsimulation = nullptr;
            }
       }

        return newsimulation;
    }

    bool SimulationPolio::ValidateConfiguration(const ::Configuration *config)
    {
        return Kernel::SimulationEnvironmental::ValidateConfiguration(config);
    }

    // called by demographic file Populate()
    void SimulationPolio::addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory)
    {
        NodePolio *node = NodePolio::CreateNode(this, node_suid);

        addNode_internal(node, nodedemographics_factory, climate_factory);
    }

    void SimulationPolio::InitializeFlags( const ::Configuration *config )
    {
    }
}

#endif // ENABLE_POLIO
