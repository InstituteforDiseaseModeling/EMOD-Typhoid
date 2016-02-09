/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "SimulationEnvironmental.h"
#include "IndividualPolio.h" // TODO: could eliminate need to include these headers if the TypedMigrationQueue template parameter was made a pointer type instead of a class type.  <ERAD-320>
#include "NodePolio.h"
#include "InfectionPolio.h"
#include "SusceptibilityPolio.h"

namespace Kernel
{
    class NodePolio;
    class IndividualHumanPolio;

    class SimulationPolio : public SimulationEnvironmental
    {
    public:
        SimulationPolio();
        static SimulationPolio *CreateSimulation();
        static SimulationPolio *CreateSimulation(const ::Configuration *config);
        virtual ~SimulationPolio(void) { }

    protected:
        static bool ValidateConfiguration(const ::Configuration *config);

        virtual void Initialize() override;
        virtual void Initialize(const ::Configuration *config) override;

        // Allows correct type of community to be added by derived class Simulations
        virtual void addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory) override;

        virtual void InitializeFlags(const ::Configuration *config) override;

    private:

        friend class Kernel::SimulationFactory; // allow them to create us
    };
}
