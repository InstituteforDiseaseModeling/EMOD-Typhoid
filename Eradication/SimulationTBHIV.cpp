/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TB

#include "Exceptions.h"
#include "SimulationTBHIV.h"
#include "NodeTBHIV.h"
#include "ReportTBHIV.h"

static const char* _module = "SimulationTBHIV";

namespace Kernel
{
    SimulationTBHIV::~SimulationTBHIV(void) { }
    SimulationTBHIV::SimulationTBHIV()
    {
        reportClassCreator = ReportTBHIV::CreateReport;
        //binnedReportClassCreator = BinnedReportTBHIV::CreateReport;
        //spatialReportClassCreator = SpatialReportTBHIV::CreateReport;
        //propertiesReportClassCreator = PropertyReportTBHIV::CreateReport;
    }

    SimulationTBHIV *SimulationTBHIV::CreateSimulation()
    {
        SimulationTBHIV *newsimulation = _new_ SimulationTBHIV();

        return newsimulation;
    }

    SimulationTBHIV *SimulationTBHIV::CreateSimulation(const ::Configuration *config)
    {
        SimulationTBHIV *newsimulation = _new_ SimulationTBHIV();
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

    bool SimulationTBHIV::ValidateConfiguration(const ::Configuration *config)
    {
        if (!SimulationAirborne::ValidateConfiguration(config))
            return false;

        // TODO: are there any more checks on configuration parameters we want to do here?

        return true;
    }

    void SimulationTBHIV::Initialize( const ::Configuration *config )
    {
        SimulationAirborne::Initialize( config );
        IndividualHumanCoinfection fakeHuman;
        LOG_INFO( "Calling Configure on fakeHuman\n" );
        fakeHuman.Configure( config );
    }

    void SimulationTBHIV::addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory)
    {
        NodeTBHIV *node = NodeTBHIV::CreateNode(this, node_suid);
        addNode_internal(node, nodedemographics_factory, climate_factory);
    }
}

#endif // ENABLE_TBHIV
