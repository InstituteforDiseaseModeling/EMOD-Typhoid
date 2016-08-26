/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <memory> // unique_ptr
#include "UnitTest++.h"
#include "INodeContextFake.h"

#include "Properties.h"
#include "Environment.h"
#include "Log.h"
#include "SimulationConfig.h"
#include "NodeDemographics.h"
#include "FileSystem.h"
#include "IdmMpi.h"

using namespace std; 
using namespace Kernel; 

SUITE(PropertiesTest)
{
    typedef boost::bimap<uint32_t, suids::suid> nodeid_suid_map_t;
    typedef nodeid_suid_map_t::value_type nodeid_suid_pair;

    struct PropertiesTestFixture
    {
        IdmMpi::MessageInterface* m_pMpi;
        SimulationConfig* pSimConfig ;
        suids::suid next_suid;

        PropertiesTestFixture()
            : pSimConfig(nullptr)
            , next_suid(suids::nil_suid())
        {
            next_suid.data++;

            Environment::setLogger(new SimpleLogger());
            int argc      = 1;
            char* exeName = "componentTests.exe";
            char** argv   = &exeName;
            string configFilename("testdata/PropertiesTest/config.json");
            string inputPath("testdata/PropertiesTest");
            string outputPath("testdata/PropertiesTest/output");
            string statePath("testdata/PropertiesTest");
            string dllPath("testdata/PropertiesTest");

            m_pMpi = IdmMpi::MessageInterface::CreateNull();

            Environment::Initialize( m_pMpi, nullptr, configFilename, inputPath, outputPath, /*statePath, */dllPath, false);

            const_cast<Environment*>(Environment::getInstance())->RNG = new PSEUDO_DES(0);

            pSimConfig = SimulationConfigFactory::CreateInstance(Environment::getInstance()->Config);
            if (pSimConfig)
            {
                Environment::setSimulationConfig(pSimConfig);
            }
            IPFactory::DeleteFactory();
            IPFactory::CreateFactory();
        }

        ~PropertiesTestFixture()
        {
            IPFactory::DeleteFactory();
            delete pSimConfig ;
            Environment::Finalize();
            NodeDemographicsFactory::SetDemographicsFileList( std::vector<std::string>() ) ;
        }

        suids::suid GetNextNodeSuid()
        {
            suids::suid next = next_suid;
            next_suid.data++;
            return next;
        }

        void TestReadingError( int lineNumber, const std::string& rDemogFilename, const std::string& rExpMsg )
        {
            // --------------------
            // --- Initialize test
            // --------------------
            pSimConfig->demographics_initial = true ;
            NodeDemographicsFactory::SetDemographicsFileList( NodeDemographicsFactory::ConvertLegacyStringToSet( rDemogFilename ) ) ;

            nodeid_suid_map_t node_id_suid_map;
            unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, Environment::getInstance()->Config, true, 10, 1000 ) );

            vector<uint32_t> nodeIDs = factory->GetNodeIDs();
            for (uint32_t node_id : nodeIDs)
            {
                suids::suid node_suid = GetNextNodeSuid();
                node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
            }

            INodeContext* nodeContext = new INodeContextFake();
            unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

            try
            {
                IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), true );
                CHECK( false );
            }
            catch( DetailedException& e )
            {
                std::string msg = e.GetMsg();
                bool passed = msg.find( rExpMsg) != std::string::npos;
                if( !passed )
                {
                    PrintDebug( msg );
                }
                CHECK_LN( passed, lineNumber );
            }
        }
    };

    TEST_FIXTURE(PropertiesTestFixture, TestRead)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        nodeid_suid_map_t node_id_suid_map;
        unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, Environment::getInstance()->Config, true, 10, 1000 ) );

        vector<uint32_t> nodeIDs = factory->GetNodeIDs();
        for (uint32_t node_id : nodeIDs)
        {
            suids::suid node_suid = GetNextNodeSuid();
            node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
        }

        try
        {
            INodeContext* nodeContext = new INodeContextFake();
            unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

            IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), true );

            std::vector<IndividualProperty*> ip_list = IPFactory::GetInstance()->GetIPList();
            CHECK_EQUAL( 2, ip_list.size() );
            CHECK_EQUAL( std::string("Accessibility"), ip_list[0]->GetKey().ToString() );
            CHECK_EQUAL( std::string("Risk"         ), ip_list[1]->GetKey().ToString() );

            IPKeyValueContainer ip_values_1 = ip_list[0]->GetValues();
            CHECK_EQUAL( 2, ip_values_1.Size() );
            CHECK( ip_values_1.Contains( "Accessibility:VaccineTake" ) );
            CHECK( ip_values_1.Contains( "Accessibility:VaccineRefuse" ) );

            CHECK( ip_list[0]->GetIntraNodeTransmissions().HasMatrix() );
            CHECK_EQUAL( std::string("contact"), ip_list[0]->GetIntraNodeTransmissions().GetRouteName() );
            CHECK_EQUAL( 2,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix().size() );
            CHECK_EQUAL( 2,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[0].size() );
            CHECK_EQUAL( 1.1f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[0][0] );
            CHECK_EQUAL( 0.3f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[0][1] );
            CHECK_EQUAL( 2,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[1].size() );
            CHECK_EQUAL( 0.3f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[1][0] );
            CHECK_EQUAL( 5.0f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[1][1] );

            IPKeyValueContainer ip_values_2 = ip_list[1]->GetValues();
            CHECK_EQUAL( 3, ip_values_2.Size() );
            CHECK(  ip_values_2.Contains( "Risk:HIGH"   ) );
            CHECK(  ip_values_2.Contains( "Risk:MEDIUM" ) );
            CHECK(  ip_values_2.Contains( "Risk:LOW"    ) );
            CHECK( !ip_values_2.Contains( "Risk:XXX"    ) );

            CHECK( !ip_list[1]->GetIntraNodeTransmissions().HasMatrix() );
            CHECK_EQUAL( std::string("contact"), ip_list[0]->GetIntraNodeTransmissions().GetRouteName() );
            CHECK_EQUAL( 0, ip_list[1]->GetIntraNodeTransmissions().GetMatrix().size() );
        }
        catch( DetailedException& e )
        {
            PrintDebug( e.GetMsg() );
            CHECK( false );
        }
    }

    TEST_FIXTURE(PropertiesTestFixture, TestReadAgeBins)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        pSimConfig->demographics_initial = true ;
        NodeDemographicsFactory::SetDemographicsFileList( NodeDemographicsFactory::ConvertLegacyStringToSet("demographics_age_bins.json") ) ;

        nodeid_suid_map_t node_id_suid_map;
        unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, Environment::getInstance()->Config, true, 10, 1000 ) );

        vector<uint32_t> nodeIDs = factory->GetNodeIDs();
        for (uint32_t node_id : nodeIDs)
        {
            suids::suid node_suid = GetNextNodeSuid();
            node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
        }

        try
        {
            INodeContext* nodeContext = new INodeContextFake();
            unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

            IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), true );

            std::vector<IndividualProperty*> ip_list = IPFactory::GetInstance()->GetIPList();
            CHECK_EQUAL( 1, ip_list.size() );
            CHECK_EQUAL( std::string("Age_Bin"), ip_list[0]->GetKey().ToString() );

            IPKeyValueContainer ip_values_1 = ip_list[0]->GetValues();
            CHECK_EQUAL( 3, ip_values_1.Size() );
            CHECK( ip_values_1.Contains( "Age_Bin:Age_Bin_Property_From_0_To_5" ) );
            CHECK( ip_values_1.Contains( "Age_Bin:Age_Bin_Property_From_5_To_13" ) );
            CHECK( ip_values_1.Contains( "Age_Bin:Age_Bin_Property_From_13_To_125" ) );

            CHECK( ip_list[0]->GetIntraNodeTransmissions().HasMatrix() );
            CHECK_EQUAL( std::string("contact"), ip_list[0]->GetIntraNodeTransmissions().GetRouteName() );
            CHECK_EQUAL( 3,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix().size() );
            CHECK_EQUAL( 3,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[0].size() );
            CHECK_EQUAL( 1.4f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[0][0] );
            CHECK_EQUAL( 1.0f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[0][1] );
            CHECK_EQUAL( 1.0f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[0][2] );
            CHECK_EQUAL( 3,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[1].size() );
            CHECK_EQUAL( 1.0f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[1][0] );
            CHECK_EQUAL( 2.5f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[1][1] );
            CHECK_EQUAL( 0.7f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[1][2] );
            CHECK_EQUAL( 3,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[2].size() );
            CHECK_EQUAL( 1.0f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[2][0] );
            CHECK_EQUAL( 0.7f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[2][1] );
            CHECK_EQUAL( 1.0f, ip_list[0]->GetIntraNodeTransmissions().GetMatrix()[2][2] );

            std::string act_filename = "testdata/PropertiesTest/output/TestReadAgeBins-Transitions-Actual.json" ;
            std::string exp_filename = "testdata/PropertiesTest/output/TestReadAgeBins-Transitions-Expected.json" ;

            // clean up before testing begnins
            CHECK( FileSystem::FileExists( exp_filename ) );

            if( FileSystem::FileExists( act_filename ) )
            {
                FileSystem::RemoveFile( act_filename );
            }

            IPFactory::GetInstance()->WriteTransitionsFile();

            CHECK( FileSystem::FileExists( "testdata/PropertiesTest/output/transitions.json" ) );

            int err = std::rename( "testdata/PropertiesTest/output/transitions.json", act_filename.c_str() );
            CHECK_EQUAL(0,err);

            CHECK( FileSystem::FileExists( act_filename ) );

            unique_ptr<string> p_act_contents( FileSystem::ReadFile( act_filename.c_str() ) );
            unique_ptr<string> p_exp_contents( FileSystem::ReadFile( exp_filename.c_str() ) );

            CHECK( *p_exp_contents == *p_act_contents );

            // clean up
            if( FileSystem::FileExists( act_filename ) )
            {
                FileSystem::RemoveFile( act_filename );
            }
        }
        catch( DetailedException& e )
        {
            PrintDebug( e.GetMsg() );
            CHECK( false );
        }
    }

    TEST_FIXTURE(PropertiesTestFixture, TestReadTransitions)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        pSimConfig->demographics_initial = true ;
        NodeDemographicsFactory::SetDemographicsFileList( NodeDemographicsFactory::ConvertLegacyStringToSet("demographics_transitions.json") ) ;

        nodeid_suid_map_t node_id_suid_map;
        unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, Environment::getInstance()->Config, true, 10, 1000 ) );

        vector<uint32_t> nodeIDs = factory->GetNodeIDs();
        for (uint32_t node_id : nodeIDs)
        {
            suids::suid node_suid = GetNextNodeSuid();
            node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
        }

        try
        {
            INodeContext* nodeContext = new INodeContextFake();
            unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

            IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), true );

            std::vector<IndividualProperty*> ip_list = IPFactory::GetInstance()->GetIPList();
            CHECK_EQUAL( 1, ip_list.size() );
            CHECK_EQUAL( std::string("QualityOfCare"), ip_list[0]->GetKey().ToString() );

            IPKeyValueContainer ip_values = ip_list[0]->GetValues();
            CHECK_EQUAL( 3, ip_values.Size() );
            CHECK( ip_values.Contains( "QualityOfCare:Good" ) );
            CHECK( ip_values.Contains( "QualityOfCare:OK"   ) );
            CHECK( ip_values.Contains( "QualityOfCare:Bad"  ) );

            CHECK( !ip_list[0]->GetIntraNodeTransmissions().HasMatrix() );
            CHECK_EQUAL( std::string("contact"), ip_list[0]->GetIntraNodeTransmissions().GetRouteName() );
            CHECK_EQUAL( 0,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix().size() );

            std::string act_filename = "testdata/PropertiesTest/output/TestReadTransitions-Transitions-Actual.json" ;
            std::string exp_filename = "testdata/PropertiesTest/output/TestReadTransitions-Transitions-Expected.json" ;

            // clean up before testing begnins
            CHECK( FileSystem::FileExists( exp_filename ) );

            if( FileSystem::FileExists( act_filename ) )
            {
                FileSystem::RemoveFile( act_filename );
            }

            IPFactory::GetInstance()->WriteTransitionsFile();

            CHECK( FileSystem::FileExists( "testdata/PropertiesTest/output/transitions.json" ) );

            int err = std::rename( "testdata/PropertiesTest/output/transitions.json", act_filename.c_str() );
            CHECK_EQUAL(0,err);

            CHECK( FileSystem::FileExists( act_filename ) );

            unique_ptr<string> p_act_contents( FileSystem::ReadFile( act_filename.c_str() ) );
            unique_ptr<string> p_exp_contents( FileSystem::ReadFile( exp_filename.c_str() ) );

            CHECK( *p_exp_contents == *p_act_contents );

            // clean up
            if( FileSystem::FileExists( act_filename ) )
            {
                FileSystem::RemoveFile( act_filename );
            }
        }
        catch( DetailedException& e )
        {
            PrintDebug( e.GetMsg() );
            CHECK( false );
        }
    }

    TEST_FIXTURE(PropertiesTestFixture, TestReadTransitionsSchool)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        pSimConfig->demographics_initial = true ;
        NodeDemographicsFactory::SetDemographicsFileList( NodeDemographicsFactory::ConvertLegacyStringToSet("demographics_transitions_school.json") ) ;

        nodeid_suid_map_t node_id_suid_map;
        unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, Environment::getInstance()->Config, true, 10, 1000 ) );

        vector<uint32_t> nodeIDs = factory->GetNodeIDs();
        for (uint32_t node_id : nodeIDs)
        {
            suids::suid node_suid = GetNextNodeSuid();
            node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
        }

        try
        {
            INodeContext* nodeContext = new INodeContextFake();
            unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

            IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), true );

            std::vector<IndividualProperty*> ip_list = IPFactory::GetInstance()->GetIPList();
            CHECK_EQUAL( 1, ip_list.size() );
            CHECK_EQUAL( std::string("Place"), ip_list[0]->GetKey().ToString() );

            IPKeyValueContainer ip_values = ip_list[0]->GetValues();
            CHECK_EQUAL( 5, ip_values.Size() );
            CHECK( ip_values.Contains( "Place:Community" ) );
            CHECK( ip_values.Contains( "Place:School"   ) );
            CHECK( ip_values.Contains( "Place:Work"  ) );
            CHECK( ip_values.Contains( "Place:SchoolBreak_Interaction"  ) );
            CHECK( ip_values.Contains( "Place:SchoolBreak_NonInteraction"  ) );

            CHECK( ip_list[0]->GetIntraNodeTransmissions().HasMatrix() );
            CHECK_EQUAL( std::string("contact"), ip_list[0]->GetIntraNodeTransmissions().GetRouteName() );
            CHECK_EQUAL( 5,    ip_list[0]->GetIntraNodeTransmissions().GetMatrix().size() );

            std::string act_filename = "testdata/PropertiesTest/output/TestReadTransitionsSchool-Transitions-Actual.json" ;
            std::string exp_filename = "testdata/PropertiesTest/output/TestReadTransitionsSchool-Transitions-Expected.json" ;

            // clean up before testing begnins
            CHECK( FileSystem::FileExists( exp_filename ) );

            if( FileSystem::FileExists( act_filename ) )
            {
                FileSystem::RemoveFile( act_filename );
            }

            IPFactory::GetInstance()->WriteTransitionsFile();

            CHECK( FileSystem::FileExists( "testdata/PropertiesTest/output/transitions.json" ) );

            int err = std::rename( "testdata/PropertiesTest/output/transitions.json", act_filename.c_str() );
            CHECK_EQUAL(0,err);

            CHECK( FileSystem::FileExists( act_filename ) );

            unique_ptr<string> p_act_contents( FileSystem::ReadFile( act_filename.c_str() ) );
            unique_ptr<string> p_exp_contents( FileSystem::ReadFile( exp_filename.c_str() ) );

            CHECK( *p_exp_contents == *p_act_contents );

            // clean up
            if( FileSystem::FileExists( act_filename ) )
            {
                FileSystem::RemoveFile( act_filename );
            }
        }
        catch( DetailedException& e )
        {
            PrintDebug( e.GetMsg() );
            CHECK( false );
        }
    }

    TEST_FIXTURE(PropertiesTestFixture, TestUninitialized)
    {
        try
        {
            IPKey key ;
            CHECK( !key.IsValid() );
            key.ToString();
            CHECK( false );
        }
        catch( NullPointerException& )
        {
            CHECK( true );
        }

        IPKeyValue kv ;
        CHECK( !kv.IsValid() );
        try
        {
            kv.ToString();
            CHECK( false );
        }
        catch( NullPointerException& )
        {
            CHECK( true );
        }

        try
        {
            kv.GetKey();
            CHECK( false );
        }
        catch( NullPointerException& )
        {
            CHECK( true );
        }

        try
        {
            kv.GetValueAsString();
            CHECK( false );
        }
        catch( NullPointerException& )
        {
            CHECK( true );
        }

        try
        {
            kv.GetInitialDistribution();
            CHECK( false );
        }
        catch( NullPointerException& )
        {
            CHECK( true );
        }
    }

    TEST_FIXTURE(PropertiesTestFixture, TestIPKeyValueContainerErrors)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/PropertiesTest/transitions_config.json" ) );
        unique_ptr<Configuration> p_parameters( Environment::CopyFromElement( (*p_config)[ "parameters" ] ) );

        nodeid_suid_map_t node_id_suid_map;
        unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, p_parameters.get(), true, 10, 1000 ) );

        vector<uint32_t> nodeIDs = factory->GetNodeIDs();
        for (uint32_t node_id : nodeIDs)
        {
            suids::suid node_suid = GetNextNodeSuid();
            node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
        }

        INodeContext* nodeContext = new INodeContextFake();
        unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

        IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), true );

        std::vector<IndividualProperty*> ip_list = IPFactory::GetInstance()->GetIPList();
        CHECK_EQUAL( 1, ip_list.size() );
        CHECK_EQUAL( std::string("QualityOfCare"), ip_list[0]->GetKey().ToString() );

        IPKeyValueContainer ip_values = ip_list[0]->GetValues();
        CHECK_EQUAL( 3, ip_values.Size() );
        CHECK( ip_values.Contains( "QualityOfCare:Good" ) );
        CHECK( ip_values.Contains( "QualityOfCare:OK"   ) );
        CHECK( ip_values.Contains( "QualityOfCare:Bad"  ) );

        try
        {
            ip_values.Get( ip_list[0]->GetKey() );
            CHECK( false );
        }
        catch( DetailedException& e )
        {
            std::string msg = e.GetMsg();
            if( msg.find( "Illegal use of IPKeyValueContainer::Get( const IPKey& rKey ).  Should not be used on containers that have multiple values for one key" ) == string::npos )
            {
                PrintDebug( e.GetMsg() );
                CHECK( false );
            }
        }

        try
        {
            IPKeyValue good( "QualityOfCare:Good" );
            ip_values.Set( good );
            CHECK( false );
        }
        catch( DetailedException& e )
        {
            std::string msg = e.GetMsg();
            if( msg.find( "Illegal use of IPKeyValueContainer::Set( const IPKeyValue& rKeyValue ).  Should not be used on containers that have multiple values for one key" ) == string::npos )
            {
                PrintDebug( e.GetMsg() );
                CHECK( false );
            }
        }
    }

    TEST_FIXTURE(PropertiesTestFixture, TestIPFactoryErrors)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        unique_ptr<Configuration> p_config( Environment::LoadConfigurationFile( "testdata/PropertiesTest/transitions_config.json" ) );
        unique_ptr<Configuration> p_parameters( Environment::CopyFromElement( (*p_config)[ "parameters" ] ) );

        nodeid_suid_map_t node_id_suid_map;
        unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, p_parameters.get(), true, 10, 1000 ) );

        vector<uint32_t> nodeIDs = factory->GetNodeIDs();
        for (uint32_t node_id : nodeIDs)
        {
            suids::suid node_suid = GetNextNodeSuid();
            node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
        }

        INodeContext* nodeContext = new INodeContextFake();
        unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

        IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), true );

        std::vector<IndividualProperty*> ip_list = IPFactory::GetInstance()->GetIPList();
        CHECK_EQUAL( 1, ip_list.size() );
        CHECK_EQUAL( std::string("QualityOfCare"), ip_list[0]->GetKey().ToString() );

        IPKeyValueContainer ip_values = ip_list[0]->GetValues();
        CHECK_EQUAL( 3, ip_values.Size() );
        CHECK( ip_values.Contains( "QualityOfCare:Good" ) );
        CHECK( ip_values.Contains( "QualityOfCare:OK"   ) );
        CHECK( ip_values.Contains( "QualityOfCare:Bad"  ) );

        try
        {
            std::string key_value_str = "invalid_key-value";
            std::string key_str;
            std::string value_str;
            IPFactory::ParseKeyValueString( key_value_str, key_str, value_str );
            CHECK( false );
        }
        catch( DetailedException& e )
        {
            std::string msg = e.GetMsg();
            if( msg.find( "Invalid Individual Property Key-Value string = 'invalid_key-value'.  Format is 'key:value'." ) == string::npos )
            {
                PrintDebug( e.GetMsg() );
                CHECK( false );
            }
        }

        try
        {
            IPFactory::GetInstance()->GetIP( "Risk", "ParamNameXXX", true );
            CHECK( false );
        }
        catch( DetailedException& e )
        {
            std::string msg = e.GetMsg();
            if( msg.find( "Could not find the IndividualProperty key = 'Risk' for parameter 'ParamNameXXX'.  Known keys are: QualityOfCare" ) == string::npos )
            {
                PrintDebug( e.GetMsg() );
                CHECK( false );
            }
        }

        try
        {
            std::string key_str = "QualityOfCare";
            std::map<std::string,float> value_dist_map;
            IPFactory::GetInstance()->AddIP( key_str, value_dist_map );
            CHECK( false );
        }
        catch( DetailedException& e )
        {
            std::string msg = e.GetMsg();
            if( msg.find( "Found existing IndividualProperty key = 'QualityOfCare'.  Can't create duplicate key.  Known keys are: QualityOfCare" ) == string::npos )
            {
                PrintDebug( e.GetMsg() );
                CHECK( false );
            }
        }
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidTransitionType)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidTransitionType.json",
                          "Invalid Individual_Property Transitions value for Type = XXX.  Known values are: At_Timestep and At_Age" );
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidAgeRestriction)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidAgeRestriction.json",
                          "Variable or parameter 'demographics[Age_In_Years_Restriction][Min]' with value 40 is incompatible with variable or parameter 'demographics[Age_In_Years_Restriction][Max]' with value 19. In the Demographics for IndividualProperties:Property=QualityOfCare, Max age must be greater than Min age.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidCoverage)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidCoverage.json",
                          "Variable Demographics[IndividualProperties][Property=QualityOfCare][Coverage] had value 999 which was inconsistent with range limit 1");
                           
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidProbability)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidProbability.json",
                          "Variable Demographics[IndividualProperties][Property=QualityOfCare][Probability_Per_Timestep] had value -1 which was inconsistent with range limit 0");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestMissingTo)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestMissingTo.json",
                          "Parameter 'To' not found in input file 'demographics file'");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestMissingStart)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestMissingStart.json",
                          "Parameter 'Start' not found in input file 'demographics file'");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidTransmissionMatrixA)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidTransmissionMatrixA.json",
                          "Invalid Transmission Matrix for property 'Age_Bin'.  It has 2 rows when it should have 3.  It should be square with one row/col per value for the property.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidTransmissionMatrixB)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidTransmissionMatrixB.json",
                          "Invalid Transmission Matrix for property 'Age_Bin'.  Row 1 has 2 columns when it should have 3.  It should be square with one row/col per value for the property.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestMissingValues)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestMissingValues.json",
                          "Failed to find Values in map demographics[IndividualProperties][0]");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestMissingInitialDistribution)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestMissingInitialDistribution.json",
                          "Failed to find Initial_Distribution in map demographics[IndividualProperties][0]");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidNumInitialDistribution)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidNumInitialDistribution.json",
                          "Number of Values in Values (2) needs to be the same as number of values in Initial_Distribution (3).");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestDuplicateValues)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestDuplicateValues.json",
                          "demographics[IndividualProperties][0] with property=Risk has a duplicate value = HIGH");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestZeroValues)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestZeroValues.json",
                          "demographics[IndividualProperties][0][Values] (property=Risk) cannot have zero values.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidInitialDistribution)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidInitialDistribution.json",
                          "The values in demographics[IndividualProperties][1][Initial_Distribution] (property=Risk) add up to 1.45.  They must add up to 1.0");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestMissingAgeBinEdge)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestMissingAgeBinEdge.json",
                          "Failed to find Age_Bin_Edges_In_Years in map demographics[IndividualProperties][0] (property = Age_Bin)");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidAgeBinEdgeA)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidAgeBinEdgeA.json",
                          "demographics[IndividualProperties][0][Age_Bin_Edges_In_Years] must have at least two values: 0 must be first and -1 is last.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidAgeBinEdgeB)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidAgeBinEdgeB.json",
                          "demographics[IndividualProperties][0][Age_Bin_Edges_In_Years] must have at least two values: 0 must be first and -1 is last.  The first value cannot be 10");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidAgeBinEdgeC)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidAgeBinEdgeC.json",
                          "demographics[IndividualProperties][0][Age_Bin_Edges_In_Years] must have at least two values: 0 must be first and -1 is last.  The last value cannot be 10");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidAgeBinEdgeD)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidAgeBinEdgeD.json",
                          "demographics[IndividualProperties][0][Age_Bin_Edges_In_Years] must be in increasing order with the last value = -1.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidAgeBinEdgeE)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidAgeBinEdgeE.json",
                          "demographics[IndividualProperties][0][Age_Bin_Edges_In_Years] must be in increasing order with the last value = -1.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestInvalidAgeBinTransition)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestInvalidAgeBinTransition.json",
                          "demographics[IndividualProperties][0][Transitions] has more than zero entries.  They are not allowed with property=Age_Bin");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestNotInWhiteList)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestNotInWhiteList.json",
                          "Invalid Individual Property key 'NonWhiteListProperty' found in demographics file. Use one of: 'Accessibility', 'Age_Bin', 'Geographic', 'HasActiveTB', 'Place', 'QualityOfCare', 'Risk'");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestTooManyProperties)
    {
        TestReadingError( __LINE__, "testdata/PropertiesTest/demog_TestTooManyProperties.json",
                          "Too many Individual Properties (4). Max is 2.");
    }

    TEST_FIXTURE(PropertiesTestFixture, TestGetAllPossibleKeyValueCombinations)
    {
        // --------------------
        // --- Initialize test
        // --------------------
        pSimConfig->demographics_initial = true ;
        NodeDemographicsFactory::SetDemographicsFileList( NodeDemographicsFactory::ConvertLegacyStringToSet( "testdata/PropertiesTest/demog_TestTooManyProperties.json" ) ) ;

        nodeid_suid_map_t node_id_suid_map;
        unique_ptr<NodeDemographicsFactory> factory( NodeDemographicsFactory::CreateNodeDemographicsFactory(&node_id_suid_map, Environment::getInstance()->Config, true, 10, 1000 ) );

        vector<uint32_t> nodeIDs = factory->GetNodeIDs();
        for (uint32_t node_id : nodeIDs)
        {
            suids::suid node_suid = GetNextNodeSuid();
            node_id_suid_map.insert(nodeid_suid_pair(node_id, node_suid));
        }

        INodeContext* nodeContext = new INodeContextFake();
        unique_ptr<NodeDemographics> demographics( factory->CreateNodeDemographics(nodeContext) );

        IPFactory::GetInstance()->Initialize( 1, demographics->GetJsonObject(), false );

        std::vector<IndividualProperty*> ip_list = IPFactory::GetInstance()->GetIPList();
        CHECK_EQUAL( 4, ip_list.size() );
        CHECK_EQUAL( std::string("Accessibility"), ip_list[0]->GetKey().ToString() );
        CHECK_EQUAL( std::string("HasActiveTB"  ), ip_list[1]->GetKey().ToString() );
        CHECK_EQUAL( std::string("QualityOfCare"), ip_list[2]->GetKey().ToString() );
        CHECK_EQUAL( std::string("Risk"         ), ip_list[3]->GetKey().ToString() );

        std::vector<std::string> combos = IPFactory::GetInstance()->GetAllPossibleKeyValueCombinations();
        //std::stringstream ss;
        //for( auto combo : combos )
        //{
        //    ss << combo << "\n";
        //}
        //PrintDebug(ss.str());

        CHECK_EQUAL( 48, combos.size() );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:E,Risk:H", combos[ 0] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:E,Risk:H", combos[ 1] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:E,Risk:H", combos[ 2] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:E,Risk:H", combos[ 3] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:G,Risk:H", combos[ 4] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:G,Risk:H", combos[ 5] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:G,Risk:H", combos[ 6] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:G,Risk:H", combos[ 7] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:O,Risk:H", combos[ 8] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:O,Risk:H", combos[ 9] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:O,Risk:H", combos[10] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:O,Risk:H", combos[11] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:B,Risk:H", combos[12] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:B,Risk:H", combos[13] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:B,Risk:H", combos[14] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:B,Risk:H", combos[15] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:E,Risk:M", combos[16] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:E,Risk:M", combos[17] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:E,Risk:M", combos[18] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:E,Risk:M", combos[19] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:G,Risk:M", combos[20] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:G,Risk:M", combos[21] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:G,Risk:M", combos[22] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:G,Risk:M", combos[23] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:O,Risk:M", combos[24] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:O,Risk:M", combos[25] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:O,Risk:M", combos[26] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:O,Risk:M", combos[27] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:B,Risk:M", combos[28] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:B,Risk:M", combos[29] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:B,Risk:M", combos[30] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:B,Risk:M", combos[31] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:E,Risk:L", combos[32] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:E,Risk:L", combos[33] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:E,Risk:L", combos[34] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:E,Risk:L", combos[35] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:G,Risk:L", combos[36] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:G,Risk:L", combos[37] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:G,Risk:L", combos[38] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:G,Risk:L", combos[39] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:O,Risk:L", combos[40] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:O,Risk:L", combos[41] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:O,Risk:L", combos[42] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:O,Risk:L", combos[43] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:F,QualityOfCare:B,Risk:L", combos[44] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:F,QualityOfCare:B,Risk:L", combos[45] );
        CHECK_EQUAL( "Accessibility:N,HasActiveTB:T,QualityOfCare:B,Risk:L", combos[46] );
        CHECK_EQUAL( "Accessibility:Y,HasActiveTB:T,QualityOfCare:B,Risk:L", combos[47] );
    }
}