/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <memory> // unique_ptr
#include "UnitTest++.h"
#include "Environment.h"
#include "SimulationConfig.h"
#include "InterventionFactory.h"
#include "JsonFullReader.h"
#include "JsonFullWriter.h"
#include "BinaryArchiveReader.h"
#include "BinaryArchiveWriter.h"
#include "JsonObjectDemog.h"
#include "Diagnostics.h"

using namespace Kernel; 

void PrintDebug( const std::string& rMessage );

SUITE(SerializationTest)
{
    static int m_NextId = 1 ;

    struct SerializationFixture
    {
        SimulationConfig* m_pSimulationConfig ;

        SerializationFixture()
        :  m_pSimulationConfig( new SimulationConfig() )
        {
            Environment::Finalize();
            Environment::setLogger( new SimpleLogger() );
            Environment::setSimulationConfig( m_pSimulationConfig );
        }

        ~SerializationFixture()
        {
            delete m_pSimulationConfig;
            m_pSimulationConfig = nullptr;
            Environment::Finalize();
        }
    };


    TEST_FIXTURE(SerializationFixture, TestSerializeDeserialize)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        try
        {
            JsonObjectDemog json ;
            json.ParseFile( "testdata/SerializationTest.json" );
            CHECK( json.Contains( "Listed_Events" ) );
            CHECK( json["Listed_Events"].IsArray() );

            for( int i = 0 ; i < json["Listed_Events"].size() ; i++ )
            {
                m_pSimulationConfig->listed_events.insert( json["Listed_Events"][i].AsString() );
            }
            CHECK( json.Contains( "Objects" ) );
            CHECK( json["Objects"].IsArray() );

            for( int i = 0 ; i < json["Objects"].size() ; i++ )
            {
                string expected_json_str = json["Objects"][i].ToString();

                ISerializable* p_read_obj = nullptr;
                JsonFullReader json_reader( expected_json_str.c_str() );
                IArchive* p_json_reader = &json_reader;
                p_json_reader->labelElement("Test") & p_read_obj;

                BinaryArchiveWriter binary_writer;
                IArchive* p_binary_writer = &binary_writer;
                p_binary_writer->labelElement("Test") & p_read_obj;

                ISerializable* p_read_obj_2 = nullptr;
                BinaryArchiveReader binary_reader( p_binary_writer->GetBuffer(), p_binary_writer->GetBufferSize() );
                IArchive* p_binary_reader = &binary_reader;
                p_binary_reader->labelElement("Test") & p_read_obj_2;

                JsonFullWriter json_writer;
                IArchive* p_json_writer = &json_writer;
                p_json_writer->labelElement("Test") & p_read_obj_2;

                std::string actual_json_str = p_json_writer->GetBuffer();

                CHECK_EQUAL( expected_json_str, actual_json_str );
            }
        }
        catch( DetailedException& re )
        {
            PrintDebug( re.GetMsg() );
            PrintDebug( re.GetStackTrace() );
            CHECK( false );
        }
    }
}
