/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Simulation.h"
#include "IIdGeneratorSTI.h"
#include "Sugar.h" // for DECLARE_VIRTUAL_BASE

namespace Kernel
{
    class IndividualHumanSTI;
    struct IRelationship;
    class SimulationSTI : public Simulation, public IIdGeneratorSTI
    {
        GET_SCHEMA_STATIC_WRAPPER(SimulationSTI)
    public:
        virtual ~SimulationSTI(void);
        static SimulationSTI *CreateSimulation();
        static SimulationSTI *CreateSimulation(const ::Configuration *config);
        SimulationSTI();

        // methods of IIdGeneratorSTI
        virtual suids::suid GetNextRelationshipSuid();

    protected:

        virtual void Initialize() override;
        virtual void Initialize(const ::Configuration *config) override;
        virtual bool Configure( const ::Configuration *json );
        virtual void Reports_CreateBuiltIn();

        static bool ValidateConfiguration(const ::Configuration *config);

        // Allows correct type of Node to be added by classes derived from Simulation
        virtual void addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory);

        suids::distributed_generator<IRelationship> relationshipSuidGenerator;

        bool report_relationship_start;
        bool report_relationship_end;
        bool report_relationship_consummated;
        bool report_transmission;
    };
}
