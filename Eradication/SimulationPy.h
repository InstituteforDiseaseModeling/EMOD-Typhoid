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
