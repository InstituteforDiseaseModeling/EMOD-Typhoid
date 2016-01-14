/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef WIN32
#include "windows.h"
#endif
#include "Debug.h"
#include "Environment.h"
#include "Configuration.h"
#include "SimulationConfig.h"

#include "Simulation.h"
#include "SimulationFactory.h"
#ifndef _DLLS_
#ifndef DISABLE_MALARIA
#include "SimulationMalaria.h"
#endif
#ifdef ENABLE_POLIO
#include "SimulationPolio.h"
#endif

#ifdef ENABLE_TB
#include "SimulationTB.h"
#endif

#ifdef ENABLE_TBHIV
#include "SimulationTBHIV.h"
#endif
#ifndef DISABLE_HIV
#include "SimulationSTI.h"
#include "SimulationHIV.h"
#endif
#endif

#include "JsonObject.h"     // for CreateJsonObjAdapter and interface declarations for .dtk file header
#include "snappy.h"         // snappy Uncompress()
#include "JsonFullReader.h" // for serialized population de-serialization

#include "DllLoader.h"

static const char* _module = "SimulationFactory";

FILE* OpenFileForReading( const char* filename )
{
    FILE* f = nullptr;
    bool opened;
    errno = 0;
#ifdef WIN32
    opened = (fopen_s( &f, filename, "rb" ) == 0);
#else
    f = fopen( filename, "rb" );
    opened = (f != nullptr);
#endif

    if ( !opened )
    {
        LOG_ERR_F( "Couldn't open serialized population file '%s' for reading (%d).\n", filename, errno );
        // TODO clorton - throw exception here, couldn't open specified serialized population file for reading
        release_assert( opened );
    }

    return f;
}

uint32_t ReadMagicNumber( FILE* f )
{
    uint32_t magic;
    size_t count = 1;
    size_t size = sizeof magic;
    size_t bytes_read = fread( &magic, count, size, f );
    if ( bytes_read != (count * size) )
    {
        LOG_ERR_F( "Only read %d bytes from serialized population file reading magic number. Wanted %d.\n", bytes_read, (count * size) );
        // TODO clorton - throw exception here
        magic = 0x0BADF00D;
        release_assert( bytes_read == (count * size) );
    }

    return magic;
}

void CheckMagicNumber( FILE* f )
{
    uint32_t magic = ReadMagicNumber( f );
    bool valid = (magic == *(uint32_t*)"IDTK");
    if (!valid)
    {
        LOG_ERR_F( "Serialized population file has wrong magic number, expected 0x%08X, got %08X.\n", *(uint32_t*)"IDTK", magic );
        // TODO clorton - throw exception here
        release_assert( valid );
    }
}

uint32_t ReadHeaderSize( FILE* f )
{
    uint32_t header_size;
    size_t count = 1;
    size_t size = sizeof header_size;
    size_t bytes_read = fread( &header_size, count, size, f );
    if ( bytes_read != (count * size) )
    {
        LOG_ERR_F( "Only read %d bytes from serialized population file reading header size. Wanted %d.\n", bytes_read, (count * size) );
        // TODO clorton - throw exception here
        header_size = 0;
        release_assert( bytes_read == (count * size) );
    }

    return header_size;
}

void CheckHeaderSize( uint32_t header_size )
{
    if ( header_size == 0 )
    {
        LOG_ERR("Serialized population header size is 0, which is not valid.\n");
        // TODO clorton - throw execption here
        release_assert( header_size );
    }
}

void ReadHeader( FILE* f, uint32_t& byte_count, bool& is_compressed )
{
    uint32_t count = ReadHeaderSize( f );
    CheckHeaderSize( count );
    char* header = (char*)malloc( count + 1 );
    release_assert( header );
    size_t size = 1;
    size_t bytes_read = fread( header, size, count, f );
    if ( bytes_read != (count * size) )
    {
        LOG_ERR_F( "Only read %d bytes from serialized population file reading header. Wanted %d.\n", bytes_read, (count * size ) );
        // TODO clorton - throw exception here
        release_assert( bytes_read == (count * size) );
    }
    header[count] = '\0';

    Kernel::IJsonObjectAdapter* adapter = Kernel::CreateJsonObjAdapter();
    adapter->Parse( header );
    // TODO clorton - wrap this with try/catch for JSON errors
    auto metadata = (*adapter)["metadata"];
    byte_count = (*metadata).GetUint( "bytecount" );
    is_compressed = (*metadata).GetBool( "compressed" );

    delete adapter;
    free( header );
}

char* ReadPayload( FILE* f, uint32_t byte_count )
{
    char* payload = (char*)malloc( byte_count );
    release_assert( payload );
    size_t bytes_read = fread( payload, 1, byte_count, f );
    if ( bytes_read != byte_count )
    {
        LOG_ERR_F( "Only read %d bytes from serialized population file reading payload. Wanted %d.\n", bytes_read, byte_count );
        // TODO clorton - throw exception here
        release_assert( bytes_read == byte_count );
    }

    return payload;
}

Kernel::ISimulation* LoadIdtkFile( const char* filename )
{
    Kernel::ISimulation* newsim = nullptr;

    FILE* f = OpenFileForReading( filename );
    CheckMagicNumber( f );
    uint32_t byte_count;
    bool is_compressed;
    ReadHeader( f, byte_count, is_compressed );

    char* payload = ReadPayload( f, byte_count );

    const char* data;
    std::string uncompressed;
    if ( is_compressed )
    {
        snappy::Uncompress( payload, byte_count, &uncompressed );
        data = uncompressed.c_str();
    }
    else
    {
        data = payload;
    }

    Kernel::IArchive* reader = static_cast<Kernel::IArchive*>(new Kernel::JsonFullReader( data ));
    (*reader).labelElement( "simulation" ) & newsim;

    delete reader;
    delete payload;

    return newsim;
}

namespace Kernel
{
    ISimulation* SimulationFactory::CreateSimulation()
    {
        ISimulation* newsim = nullptr;

        if ( CONFIG_PARAMETER_EXISTS( EnvPtr->Config, "Serialized_Population_Filename" ) )
        {
            std::string filename = GET_CONFIG_STRING( EnvPtr->Config, "Serialized_Population_Filename" );
            newsim = LoadIdtkFile( filename.c_str() );
            newsim->Initialize( EnvPtr->Config );
            return newsim;
        }

        std::string sSimType;

        try
        {
            sSimType = GET_CONFIG_STRING(EnvPtr->Config, "Simulation_Type");

            SimType::Enum sim_type;
            if (sSimType == "GENERIC_SIM")
                sim_type = SimType::GENERIC_SIM;
#ifndef DISABLE_VECTOR
            else if (sSimType == "VECTOR_SIM")
                sim_type = SimType::VECTOR_SIM;
#endif
#ifndef DISABLE_MALARIA
            else if (sSimType == "MALARIA_SIM")
                sim_type = SimType::MALARIA_SIM;
#endif
#ifdef ENABLE_POLIO
            else if (sSimType == "ENVIRONMENTAL_SIM")
                sim_type = SimType::ENVIRONMENTAL_SIM;
            else if (sSimType == "POLIO_SIM")
                sim_type = SimType::POLIO_SIM;
#endif
#ifdef ENABLE_TB
            else if (sSimType == "AIRBORNE_SIM")
                sim_type = SimType::AIRBORNE_SIM;
            else if (sSimType == "TB_SIM")
                sim_type = SimType::TB_SIM;
#ifdef ENABLE_TBHIV
            else if (sSimType == "TBHIV_SIM")
                sim_type = SimType::TBHIV_SIM;
#endif // TBHIV
#endif // TB
#ifndef DISABLE_HIV
            else if (sSimType == "STI_SIM")
                sim_type = SimType::STI_SIM;
            else if (sSimType == "HIV_SIM")
                sim_type = SimType::HIV_SIM;
#endif // HIV
            else
            {
                std::ostringstream msg;
                msg << "Simulation_Type " << sSimType << " not recognized." << std::endl;
                throw Kernel::GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }

#ifdef _DLLS_

            // Look through disease dll directory, do LoadLibrary on each .dll,
            // do GetProcAddress on GetMimeType() and CreateSimulation
            typedef ISimulation* (*createSim)(const Environment *);
            std::map< std::string, createSim > createSimFuncPtrMap;

            // Note map operator [] will automatically initialize the pointer to NULL if not found
            DllLoader dllLoader;         
            if (!dllLoader.LoadDiseaseDlls(createSimFuncPtrMap) || !createSimFuncPtrMap[sSimType])
            {
                std::ostringstream msg;
                msg << "Failed to load disease emodules for SimType: " << SimType::pairs::lookup_key(sim_type) << " from path: " << dllLoader.GetEModulePath(DISEASE_EMODULES).c_str() << std::endl;
                throw Kernel::DllLoadingException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str());
                return newsim;
            }
            newsim = createSimFuncPtrMap[sSimType](EnvPtr);
            release_assert(newsim);

#else // _DLLS_
            switch (sim_type)
            {
                case SimType::GENERIC_SIM:
                    newsim = Simulation::CreateSimulation(EnvPtr->Config);
                break;
#ifdef ENABLE_POLIO
                case SimType::ENVIRONMENTAL_SIM:
                    newsim = SimulationEnvironmental::CreateSimulation(EnvPtr->Config);
                break;

                case SimType::POLIO_SIM:
                    newsim = SimulationPolio::CreateSimulation(EnvPtr->Config);
                break;
#endif        
#ifndef DISABLE_VECTOR
                case SimType::VECTOR_SIM:
                    newsim = SimulationVector::CreateSimulation(EnvPtr->Config);
                break;
#endif
#ifndef DISABLE_MALARIA
                case SimType::MALARIA_SIM:
                    newsim = SimulationMalaria::CreateSimulation(EnvPtr->Config);
                break;
#endif
#ifdef ENABLE_TB
                case SimType::AIRBORNE_SIM:
                    newsim = SimulationAirborne::CreateSimulation(EnvPtr->Config);
                break;

                case SimType::TB_SIM:
                    newsim = SimulationTB::CreateSimulation(EnvPtr->Config);
                break;
#ifdef ENABLE_TBHIV
                case SimType::TBHIV_SIM:
                    newsim = SimulationTBHIV::CreateSimulation(EnvPtr->Config);
                break;
#endif // TBHIV
#endif // TB
#ifndef DISABLE_HIV 
                case SimType::STI_SIM:
                    newsim = SimulationSTI::CreateSimulation(EnvPtr->Config);
                break;

                case SimType::HIV_SIM:
                    newsim = SimulationHIV::CreateSimulation(EnvPtr->Config);
                break;
#endif // HIV

                default: 
                // Is it even possible to get here anymore?  Won't the SimulationConfig error out earlier parsing the parameter-string?
                //("SimulationFactory::CreateSimulation(): Error, Simulation_Type %d is not implemented.\n", sim_type);
                //throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "sim_type", sim_type, SimType::pairs::lookup_key( sim_typ )); // JB
                break;
            }
#endif
            release_assert(newsim);
        }
        catch ( GeneralConfigurationException& e ) {
            LOG_ERR(e.GetMsg());
            LOG_ERR("Caught GeneralConfigurationException trying to CreateSimulation(). Returning NULL for newsim.\n");
        }

        return newsim;
    }

}
