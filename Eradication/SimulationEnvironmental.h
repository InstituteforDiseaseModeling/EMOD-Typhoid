/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "SimulationFactory.h"
#include "Simulation.h"

namespace Kernel
{
    class SimulationEnvironmental : public Simulation
    {
    public:
        static SimulationEnvironmental *CreateSimulation();
        static SimulationEnvironmental *CreateSimulation(const ::Configuration *config);
        virtual ~SimulationEnvironmental(void);

    protected:
        SimulationEnvironmental();
        /* clorton virtual */ void Initialize() /* clorton override */;
        /* clorton virtual */ void Initialize(const ::Configuration *config) /* clorton override */;

        static bool ValidateConfiguration(const ::Configuration *config);

        // Allows correct type of Node to be added by derived class Simulations
        virtual void addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory) override;

        virtual void InitializeFlags(const ::Configuration *config);

    private:
        friend class Kernel::SimulationFactory; // allow them to create us
    };
}
