/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/


#include "stdafx.h"
#include "UnitTest++.h"
#include <string>
#include <memory>

#include "MigrateIndividuals.h"
//#include "IndividualHumanContextFake.h"
//#include "IndividualHumanInterventionsContextFake.h"
//#include "ICampaignCostObserverFake.h"
//#include "INodeContextFake.h"
//#include "INodeEventContextFake.h"
//#include "ISusceptibilityHIVFake.h"

#include "FileSystem.h"
#include "Configuration.h"
//#include "Simulation.h"
//#include "Node.h"
#include "SimulationConfig.h"

using namespace Kernel;

SUITE(MigrateIndividualsTest)
{
    struct MigrateFixture
    {
        MigrateIndividuals  m_MigrateIndividuals;
        SimulationConfig*   m_pSimulationConfig ;

        MigrateFixture()
            : m_MigrateIndividuals()
            , m_pSimulationConfig( new SimulationConfig() )
        {
            Environment::Finalize();
            Environment::setLogger( new SimpleLogger( Logger::tLevel::WARNING ) );
            Environment::setSimulationConfig( m_pSimulationConfig );

            m_pSimulationConfig->migration_structure = MigrationStructure::NO_MIGRATION;
        }

        ~MigrateFixture()
        {
            delete m_pSimulationConfig;
            Environment::Finalize();
        }
    };

    TEST_FIXTURE(MigrateFixture, TestPositive)
    {
        // -------------------------------------------------------------
        // --- Test that the intervention Configures() ok when
        // --- the migration_structure is valid.
        // -------------------------------------------------------------
        m_pSimulationConfig->migration_structure = MigrationStructure::FIXED_RATE_MIGRATION;
        std::unique_ptr<Configuration> p_config( Configuration_Load( "testdata/MigrateIndividualsTest.json" ) );
        m_MigrateIndividuals.Configure( p_config.get() );
    }

    TEST_FIXTURE(MigrateFixture, TestNegative)
    {
        // -------------------------------------------------------------
        // --- Test that the intervention Configures() ok when
        // --- the migration_structure is valid.
        // -------------------------------------------------------------
        try
        {
            m_pSimulationConfig->migration_structure = MigrationStructure::NO_MIGRATION;
            std::unique_ptr<Configuration> p_config( Configuration_Load( "testdata/MigrateIndividualsTest.json" ) );
            m_MigrateIndividuals.Configure( p_config.get() );
            CHECK( false ); // should not get here
        }
        catch( DetailedException& re )
        {
            std::string msg = re.GetMsg();
            bool passed = msg.find( "MigrateIndividuals cannot be used when 'Migration_Model' = 'NO_MIGRATION'." ) != string::npos ;
            if( !passed )
            {
                PrintDebug( msg );
            }
            CHECK( passed );
        }

    }
}
