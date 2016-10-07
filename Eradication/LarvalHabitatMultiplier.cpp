/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "LarvalHabitatMultiplier.h"
#include "SimulationConfig.h"
#include "JsonObjectDemog.h"
#include "Log.h"
#include "VectorParameters.h"

static const char* _module = "LarvalHabitatMultiplier";

namespace Kernel
{
    LarvalHabitatMultiplier::LarvalHabitatMultiplier( float minValue, float maxValue, float defaultValue )
    : JsonConfigurable()
    , m_MinValue( minValue )
    , m_MaxValue( maxValue )
    , m_DefaultValue( defaultValue )
    , m_Initialized(false)
    , m_Multiplier()
    {
    }

    LarvalHabitatMultiplier::~LarvalHabitatMultiplier()
    {
    }

    void LarvalHabitatMultiplier::Initialize()
    {
        if( JsonConfigurable::_dryrun ) return;

        m_Multiplier.clear();

        for( int i = 0 ; i < VectorHabitatType::pairs::count(); ++i )
        {
            std::map<std::string,float> species_map;
            VectorHabitatType::Enum vht = VectorHabitatType::Enum( VectorHabitatType::pairs::get_values()[i] );

            for( auto& species : GET_CONFIGURABLE(SimulationConfig)->vector_params->vector_species_names )
            {
                species_map.insert( std::make_pair( species, m_DefaultValue ) );
            }

            m_Multiplier.insert( std::make_pair( vht, species_map ) );
        }
        m_Initialized = true;
    }

    bool LarvalHabitatMultiplier::WasInitialized() const
    {
        return m_Initialized;
    }

    float LarvalHabitatMultiplier::GetMultiplier( VectorHabitatType::Enum vht, const std::string& species ) const
    {
        return m_Multiplier.at( vht ).at( species );
    }

    void LarvalHabitatMultiplier::SetMultiplier( VectorHabitatType::Enum vht, float multiplier )
    {
        std::map<std::string,float>& species_map = m_Multiplier[ vht ];
        for( auto& species_entry : species_map )
        {
            species_entry.second = multiplier;
        }
    }

    void LarvalHabitatMultiplier::SetAsReduction( const LarvalHabitatMultiplier& rRegularLHM )
    {
        release_assert( this->m_Multiplier.size() == rRegularLHM.m_Multiplier.size() );
        for( auto& vht_entry : m_Multiplier )
        {
            release_assert( vht_entry.second.size() == rRegularLHM.m_Multiplier.at(vht_entry.first).size() );
            std::map<std::string,float>& species_map = vht_entry.second;
            for( auto& species_entry : species_map )
            {
                // ---------------------------------------------------------------------------------
                // --- If we are calling this method, then "this" object must be used for reduction,
                // --- so we convert the values from the standard multiplier
                // ---------------------------------------------------------------------------------
                species_entry.second = 1.0 - rRegularLHM.m_Multiplier.at(vht_entry.first).at(species_entry.first);
            }
        }
    }

    void LarvalHabitatMultiplier::Read( const JsonObjectDemog& rJsonData, uint32_t externalNodeId )
    {
        Initialize();

        if( rJsonData.IsObject() )
        {
            for( auto& vht_entry : m_Multiplier )
            {
                std::string habitat_name = VectorHabitatType::pairs::lookup_key( vht_entry.first );
                std::map<std::string,float>& species_map = vht_entry.second;

                if( rJsonData.Contains( habitat_name.c_str() ) )
                {
                    VectorHabitatType::Enum habitat = vht_entry.first;
                    if( rJsonData[ habitat_name.c_str() ].IsObject() )
                    {
                        for( auto& species_entry : species_map )
                        {
                            const std::string& species_name = species_entry.first;
                            float multiplier = m_DefaultValue;
                            if( rJsonData[ habitat_name.c_str() ].Contains( species_name.c_str() ) )
                            {
                                multiplier = float(rJsonData[ habitat_name.c_str() ][ species_name.c_str() ].AsDouble());
                                CheckRange( multiplier, externalNodeId, habitat_name, species_name );
                            }
                            species_entry.second = multiplier;
                            LOG_INFO_F("Node ID=%d with LarvalHabitatMultiplier(%s)(%s)=%0.2f\n", externalNodeId, habitat_name.c_str(), species_name.c_str(), multiplier);
                        }
                    }
                    else
                    {
                        float multiplier = float(rJsonData[ habitat_name.c_str() ].AsDouble());
                        CheckRange( multiplier, externalNodeId, habitat_name, "" );
                        for( auto& species_entry : species_map )
                        {
                            species_entry.second = multiplier;
                            LOG_INFO_F("Node ID=%d with LarvalHabitatMultiplier(%s)(%s)=%0.2f\n", externalNodeId, habitat_name.c_str(), species_entry.first.c_str(), multiplier);
                        }
                    }
                }
                else
                {
                    LOG_DEBUG_F("No LarvalHabitatMultiplier specified for %s habitat at Node ID=%d\n",habitat_name.c_str(),externalNodeId);
                }
            }
        }
        else
        {
            float multiplier = float(rJsonData.AsDouble());
            CheckRange( multiplier, externalNodeId, "", "" );

            std::map<std::string,float>& species_map = m_Multiplier[ VectorHabitatType::ALL_HABITATS ];

            for( auto& species_entry : species_map )
            {
                species_entry.second = multiplier;
            }
            LOG_INFO_F("Node ID=%d with LarvalHabitatMultiplier(ALL_HABITATS)=%0.2f\n", externalNodeId, multiplier);
            LOG_WARN("DeprecationWarning: Specification of \"LarvalHabitatMultiplier\" as a floating-point value in the \"NodeAttributes\" block will soon be deprecated. Specify as an object with habitat-type keys, e.g. \"LarvalHabitatMultiplier\" : {\"TEMPORARY_RAINFALL\" : 0.3}\n");
        }
    }

    void LarvalHabitatMultiplier::CheckRange( float multiplier, 
                                              uint32_t externalNodeId, 
                                              const std::string& rHabitatName, 
                                              const std::string& rSpeciesName )
    {
        if( (multiplier < m_MinValue) || (m_MaxValue < multiplier) )
        {
            std::stringstream ss;
            ss << "Invalid Larval Habitat Multipler = " << multiplier << " - minimum=" << m_MinValue << ", maximum=" << m_MaxValue << "\n";
            ss << "Found while reading data for: ";
            if( externalNodeId > 0 )
            {
                ss << "NodeID=" << externalNodeId << ", ";
            }
            if( !rHabitatName.empty() )
            {
                ss << "Habitat_Type=" << rHabitatName << ", ";
            }
            if( !rSpeciesName.empty() )
            {
                ss << "Species_Name=" << rSpeciesName;
            }
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }
    }


    void LarvalHabitatMultiplier::ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key )
    {
        if( !inputJson->Exist( key ) )
        {
            LOG_DEBUG_F("Could not find json element '%s'.  Did not initialize.\n",key.c_str());
            return;
        }

        std::stringstream json_data_in_string;
        json::Writer::Write( (*inputJson)[key], json_data_in_string );
        //printf("%s\n",json_data_in_string.str().c_str());

        JsonObjectDemog json_data;
        try
        {
            json_data.Parse( json_data_in_string.str().c_str() );
        }
        catch( DetailedException& )
        {
            std::stringstream ss;
            ss << "Error reading/parsing '" << key << "' from campaign file.";
            throw SerializationException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }
        Read( json_data, 0 );
    }

    json::QuickBuilder LarvalHabitatMultiplier::GetSchema()
    {
        json::QuickBuilder schema( GetSchemaBase() );
        auto tn = JsonConfigurable::_typename_label();
        auto ts = JsonConfigurable::_typeschema_label();
        schema[ tn ] = json::String( "idmType:LarvalHabitatMultiplier" );
        //schema[ts] = json::Array();
        //schema[ts][0] = json::Object();
        //schema[ts][0]["Low"] = json::Object();
        //schema[ts][0]["Low"][ "type" ] = json::String( "float" );
        //schema[ts][0]["Low"][ "min" ] = json::Number( 0 );
        //schema[ts][0]["Low"][ "max" ] = json::Number( 1000.0 );
        //schema[ts][0]["Low"][ "description" ] = json::String( HIV_Age_Diagnostic_Low_DESC_TEXT );
        //schema[ts][0]["High"] = json::Object();
        //schema[ts][0]["High"][ "type" ] = json::String( "float" );
        //schema[ts][0]["High"][ "min" ] = json::Number( 0 );
        //schema[ts][0]["High"][ "max" ] = json::Number( 1000.0 );
        //schema[ts][0]["High"][ "description" ] = json::String( HIV_Age_Diagnostic_High_DESC_TEXT );
        //schema[ts][0]["Event"] = json::Object();
        //schema[ts][0]["Event"][ "type" ] = json::String( "String" );
        //schema[ts][0]["Event"][ "description" ] = json::String( HIV_Age_Diagnostic_Event_Name_DESC_TEXT );
        return schema;
    }
}
