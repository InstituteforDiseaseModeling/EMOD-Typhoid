/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "SimulationEnvironmental.h"
#include "IndividualPy.h" // TODO: could eliminate need to include these headers if the TypedMigrationQueue template parameter was made a pointer type instead of a class type.  <ERAD-320>
#include "NodePy.h"
#include "InfectionPy.h"
#include "SusceptibilityPy.h"

namespace Kernel
{
    class NodePy;
    class IndividualHumanPy;

    class SimulationPy : public Simulation
    {
    public:
        SimulationPy();
        static SimulationPy *CreateSimulation();
        static SimulationPy *CreateSimulation(const ::Configuration *config);
        virtual ~SimulationPy(void) { }
        virtual void Reports_CreateBuiltIn();

    protected:
        static bool ValidateConfiguration(const ::Configuration *config);

        void Initialize();
        void Initialize(const ::Configuration *config);

        // Allows correct type of community to be added by derived class Simulations
        virtual void addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory);

        virtual void InitializeFlags(const ::Configuration *config);
        virtual void resolveMigration();

    private:

        friend class Kernel::SimulationFactory; // allow them to create us

#if USE_BOOST_SERIALIZATION
        template<class Archive>
        friend void serialize(Archive & ar, SimulationPy& sim, const unsigned int  file_version );
#endif
        //TypedPrivateMigrationQueueStorage<IndividualHumanPy> typed_migration_queue_storage;
    };
}

#ifndef WIN32
//DECLARE_VIRTUAL_BASE_OF(Kernel::Simulation, Kernel::SimulationPy)
#endif
