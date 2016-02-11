/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_PYTHON

#include "SimulationPyDemo.h"
#include "NodePyDemo.h"
#include "IndividualPyDemo.h"
#include "InfectionPyDemo.h"
#include "SusceptibilityPyDemo.h"
#include "suids.hpp"
#include "ReportPyDemo.h"
#include "BinnedReportPyDemo.h"
#include "SpatialReportPyDemo.h"
#include "ProgVersion.h"

#pragma warning(disable : 4996)

static const char * _module = "SimulationPyDemo";

#ifdef _TYPHOID_DLL

// Note: _diseaseType has to match the Simulation_Type name in config.json
static std::string _diseaseType = "TYPHOID_SIM";

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
    return Kernel::SimulationPyDemo::CreateSimulation( EnvPtr->Config );
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
    SimulationPyDemo::SimulationPyDemo() : Simulation()
    {
        reportClassCreator = ReportPyDemo::CreateReport;
        binnedReportClassCreator = BinnedReportPyDemo::CreateReport;
        spatialReportClassCreator = SpatialReportPyDemo::CreateReport;
    }

    void SimulationPyDemo::Initialize()
    {
        Simulation::Initialize();
    }

    void SimulationPyDemo::Initialize(const ::Configuration *config)
    {
        Simulation::Initialize(config);
        IndividualHumanPyDemo fakeHuman;
        LOG_INFO( "Calling Configure on fakeHuman\n" );
        fakeHuman.Configure( config );
    }

    SimulationPyDemo *SimulationPyDemo::CreateSimulation()
    {
        SimulationPyDemo *newsimulation = _new_ SimulationPyDemo();
        newsimulation->Initialize();

        return newsimulation;
    }

    SimulationPyDemo *SimulationPyDemo::CreateSimulation(const ::Configuration *config)
    {
       SimulationPyDemo *newsimulation = NULL;
       
       newsimulation = _new_ SimulationPyDemo();
       if (newsimulation)
       {
            // This sequence is important: first
            // Creation-->Initialization-->Validation
            newsimulation->Initialize(config);
            if(!ValidateConfiguration(config))
            {
                delete newsimulation;
                newsimulation = NULL;
            }
       }

        return newsimulation;
    }

    bool SimulationPyDemo::ValidateConfiguration(const ::Configuration *config)
    {
        return Kernel::Simulation::ValidateConfiguration(config);
    }

    // called by demographic file Populate()
    void SimulationPyDemo::addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory)
    {
        NodePyDemo *node = NodePyDemo::CreateNode(this, node_suid);

        addNode_internal(node, nodedemographics_factory, climate_factory);
    }

    void SimulationPyDemo::InitializeFlags( const ::Configuration *config )
    {
    }


    void SimulationPyDemo::resolveMigration()
    {
        resolveMigrationInternal( typed_migration_queue_storage, migratingIndividualQueues );
    }

    void SimulationPyDemo::Reports_CreateBuiltIn()
    {
        return Simulation::Reports_CreateBuiltIn();
    }
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(Kernel::SimulationPyDemo)
namespace Kernel {
    template<class Archive>
    void serialize(Archive & ar, SimulationPyDemo& sim, const unsigned int  file_version )
    {
        // must register all derived type serialized through base class pointer members
        ar.template register_type<NodePyDemo>();
        ar.template register_type<SimulationPyDemoFlags>();
        ar.template register_type<NodePyDemoFlags>();
        ar.template register_type<IndividualHumanPyDemoFlags>();
        ar.template register_type<InfectionPyDemoFlags>();
        ar.template register_type<SusceptibilityPyDemoFlags>();

        ar & boost::serialization::base_object<Simulation>(sim);
    }
}
#endif

#endif // 
