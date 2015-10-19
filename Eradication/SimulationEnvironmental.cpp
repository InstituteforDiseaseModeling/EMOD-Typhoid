/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "SimulationEnvironmental.h"
#include "NodeEnvironmental.h"
#include "InfectionEnvironmental.h"
#include "SusceptibilityEnvironmental.h"
//#include "ReportEnvironmental.h"
#include "SimulationConfig.h"
#include "ProgVersion.h"

using namespace Kernel;

#pragma warning(disable : 4996)

static const char * _module = "SimulationEnvironmental";

namespace Kernel
{
    SimulationEnvironmental::SimulationEnvironmental() : Simulation()
    {
        // not setting reporter creator function pointer because there's no actual
        // unique functionality for Environmental
    }

    SimulationEnvironmental::~SimulationEnvironmental(void)
    {
    }

    void SimulationEnvironmental::Initialize()
    {
        Simulation::Initialize();
    }

    void SimulationEnvironmental::Initialize(const ::Configuration *config)
    {
        Simulation::Initialize(config);
    }

    SimulationEnvironmental *SimulationEnvironmental::CreateSimulation()
    {
        SimulationEnvironmental *newsimulation = _new_ SimulationEnvironmental();
        newsimulation->Initialize();

        return newsimulation;
    }

    SimulationEnvironmental *SimulationEnvironmental::CreateSimulation(const ::Configuration *config)
    {
        SimulationEnvironmental *newsimulation = NULL;

        newsimulation = _new_ SimulationEnvironmental();
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

    bool SimulationEnvironmental::ValidateConfiguration(const ::Configuration *config)
    {
        if (!Kernel::Simulation::ValidateConfiguration(config))
            return false;

        // TODO: any disease-specific validation goes here.
        // Warning: static climate parameters are not configured until after this function is called

        return true;
    }
}

void SimulationEnvironmental::addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory)
{
    NodeEnvironmental *node = NodeEnvironmental::CreateNode(this, node_suid);

    addNode_internal(node, nodedemographics_factory, climate_factory);
}

void Kernel::SimulationEnvironmental::InitializeFlags( const ::Configuration *config )
{
}

#include "IndividualEnvironmental.h"
void Kernel::SimulationEnvironmental::resolveMigration()
{
    resolveMigrationInternal( typed_migration_queue_storage, migratingIndividualQueues );
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(Kernel::SimulationEnvironmental)
namespace Kernel {
    template<class Archive>
    void serialize(Archive & ar, SimulationEnvironmental &sim, const unsigned int  file_version )
    {
        ar.template register_type<NodeEnvironmental>();
        ar.template register_type<NodeEnvironmentalFlags>();

        ar & boost::serialization::base_object<Simulation>(sim);
    }
}
#endif

#endif // ENABLE_POLIO
