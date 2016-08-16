/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "UnitTest++.h"
#include "common.h"
#include <memory> // unique_ptr


#include "FileSystem.h"
#include "SimulationConfig.h"
#include "JsonObjectDemog.h"
#include "IdmMpi.h"

#include "LarvalHabitatMultiplier.h"

using namespace Kernel;
using namespace std;



SUITE(LarvalHabitatMultiplierTest)
{
    struct LhmFixture
    {
        SimulationConfig* m_pSimulationConfig ;

        LhmFixture()
        : m_pSimulationConfig( new SimulationConfig() )
        {
            JsonConfigurable::ClearMissingParameters();

            Environment::Finalize();
            Environment::setLogger( new SimpleLogger( Logger::tLevel::WARNING ) );
            Environment::setSimulationConfig( m_pSimulationConfig );

            m_pSimulationConfig->vector_species_names.insert( "arabiensis" );
            m_pSimulationConfig->vector_species_names.insert( "funestus" );
            m_pSimulationConfig->vector_species_names.insert( "gambiae" );
        }

        ~LhmFixture()
        {
            delete m_pSimulationConfig;
            Environment::Finalize();
        }
    };

    void CheckOneValue( LarvalHabitatMultiplier& lhm )
    {
    }

    TEST_FIXTURE(LhmFixture, TestReadOneValueA)
    {
        JsonObjectDemog json;
        json.ParseFile( "testdata/LarvalHabitatMultiplierTest/TestReadOneValue.json" );

        LarvalHabitatMultiplier lhm;

        lhm.Read( json["LarvalHabitatMultiplier"], 789 );

        for( int i = 0 ; i < VectorHabitatType::pairs::count() ; ++i )
        {
            VectorHabitatType::Enum vht = VectorHabitatType::Enum( VectorHabitatType::pairs::get_values()[i] );
            if( vht == VectorHabitatType::ALL_HABITATS )
            {
                // Only ALL_HABITATS gets the value in the file
                CHECK_CLOSE( 1.23, lhm.GetMultiplier( VectorHabitatType::ALL_HABITATS,   "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 1.23, lhm.GetMultiplier( VectorHabitatType::ALL_HABITATS,   "funestus"   ), 0.0001 );
                CHECK_CLOSE( 1.23, lhm.GetMultiplier( VectorHabitatType::ALL_HABITATS,   "gambiae"    ), 0.0001 );
            }
            else
            {
                // The other types just get the default value
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
        }
    }

    TEST_FIXTURE(LhmFixture, TestReadOneValueB)
    {
        std::unique_ptr<Configuration> p_config( Configuration_Load( "testdata/LarvalHabitatMultiplierTest/TestReadOneValue.json" ) );

        LarvalHabitatMultiplier lhm;

        try
        {
            lhm.ConfigureFromJsonAndKey( p_config.get(), "LarvalHabitatMultiplier" );
            CHECK( false );
        }
        catch( SerializationException&  )
        {
            // invalid to read the one line value in the campaign file
            CHECK( true );
        }
    }


    void CheckReadHabitatValue( LarvalHabitatMultiplier& lhm )
    {
        for( int i = 0 ; i < VectorHabitatType::pairs::count() ; ++i )
        {
            VectorHabitatType::Enum vht = VectorHabitatType::Enum( VectorHabitatType::pairs::get_values()[i] );
            if( vht == VectorHabitatType::BRACKISH_SWAMP )
            {
                CHECK_CLOSE( 2.0, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 2.0, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 2.0, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
            else if( vht == VectorHabitatType::HUMAN_POPULATION )
            {
                CHECK_CLOSE( 3.0, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 3.0, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 3.0, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
            else
            {
                // The other types just get the default value
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
        }
    }

    TEST_FIXTURE(LhmFixture, TestReadHabitatValueA)
    {
        JsonObjectDemog json;
        json.ParseFile( "testdata/LarvalHabitatMultiplierTest/TestReadHabitatValue.json" );

        LarvalHabitatMultiplier lhm;

        lhm.Read( json["LarvalHabitatMultiplier"], 789 );

        CheckReadHabitatValue( lhm );
    }

    TEST_FIXTURE(LhmFixture, TestReadHabitatValueB)
    {
        std::unique_ptr<Configuration> p_config( Configuration_Load( "testdata/LarvalHabitatMultiplierTest/TestReadHabitatValue.json" ) );

        LarvalHabitatMultiplier lhm;

        lhm.ConfigureFromJsonAndKey( p_config.get(), "LarvalHabitatMultiplier" );

        CheckReadHabitatValue( lhm );
    }


    void CheckReadHabitatSpeciesValue( LarvalHabitatMultiplier& lhm )
    {
        for( int i = 0 ; i < VectorHabitatType::pairs::count() ; ++i )
        {
            VectorHabitatType::Enum vht = VectorHabitatType::Enum( VectorHabitatType::pairs::get_values()[i] );
            if( vht == VectorHabitatType::LINEAR_SPLINE )
            {
                CHECK_CLOSE( 2.0, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 3.0, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 1.0, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
            else if( vht == VectorHabitatType::TEMPORARY_RAINFALL )
            {
                CHECK_CLOSE( 1.0, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 1.0, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 7.0, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
            else
            {
                // The other types just get the default value
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
        }
    }


    TEST_FIXTURE(LhmFixture, TestReadHabitatSpeciesValueA)
    {
        JsonObjectDemog json;
        json.ParseFile( "testdata/LarvalHabitatMultiplierTest/TestReadHabitatSpeciesValue.json" );

        LarvalHabitatMultiplier lhm;

        lhm.Read( json["LarvalHabitatMultiplier"], 789 );

        CheckReadHabitatSpeciesValue( lhm );
    }

    TEST_FIXTURE(LhmFixture, TestReadHabitatSpeciesValueB)
    {
        std::unique_ptr<Configuration> p_config( Configuration_Load( "testdata/LarvalHabitatMultiplierTest/TestReadHabitatSpeciesValue.json" ) );

        LarvalHabitatMultiplier lhm;

        lhm.ConfigureFromJsonAndKey( p_config.get(), "LarvalHabitatMultiplier" );

        CheckReadHabitatSpeciesValue( lhm );
    }



    void CheckReadMix( LarvalHabitatMultiplier& lhm )
    {
        for( int i = 0 ; i < VectorHabitatType::pairs::count() ; ++i )
        {
            VectorHabitatType::Enum vht = VectorHabitatType::Enum( VectorHabitatType::pairs::get_values()[i] );
            if( vht == VectorHabitatType::TEMPORARY_RAINFALL )
            {
                // set by habitat type
                CHECK_CLOSE( 5.0, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 5.0, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 5.0, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
            else if( vht == VectorHabitatType::WATER_VEGETATION )
            {
                // set by species
                CHECK_CLOSE( 6.0, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 1.0, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 7.0, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
            else
            {
                // The other types just get the default value
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "arabiensis" ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "funestus"   ), 0.0001 );
                CHECK_CLOSE( 1.00, lhm.GetMultiplier( vht, "gambiae"    ), 0.0001 );
            }
        }
    }

    TEST_FIXTURE(LhmFixture, TestReadMixA)
    {
        JsonObjectDemog json;
        json.ParseFile( "testdata/LarvalHabitatMultiplierTest/TestReadMix.json" );

        LarvalHabitatMultiplier lhm;

        lhm.Read( json["LarvalHabitatMultiplier"], 789 );

        CheckReadMix( lhm );
    }

    TEST_FIXTURE(LhmFixture, TestReadMixB)
    {
        std::unique_ptr<Configuration> p_config( Configuration_Load( "testdata/LarvalHabitatMultiplierTest/TestReadMix.json" ) );

        LarvalHabitatMultiplier lhm;

        lhm.ConfigureFromJsonAndKey( p_config.get(), "LarvalHabitatMultiplier" );

        CheckReadMix( lhm );
    }

    TEST_FIXTURE(LhmFixture, TestInvalidValue)
    {
        JsonObjectDemog json;
        json.ParseFile( "testdata/LarvalHabitatMultiplierTest/TestInvalidValue.json" );

        LarvalHabitatMultiplier lhm;

        try
        {
            lhm.Read( json["LarvalHabitatMultiplier"], 789 );
            CHECK( false );
        }
        catch( DetailedException& re )
        {
            PrintDebug( re.GetMsg() );
            CHECK( true );
        }
    }

}
