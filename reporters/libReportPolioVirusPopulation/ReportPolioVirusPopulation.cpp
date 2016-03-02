/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "StdAfx.h"

#include <fstream>

#include "ReportPolioVirusPopulation.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"
#include "IdmMpi.h"

#include "Environment.h"
#include "INodeContext.h"
#include "IndividualPolio.h"
#include "InfectionPolio.h"
#include "Log.h"
#include "PolioContexts.h"

using namespace std;
using namespace json;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Module name for logging, CustomReport.json, and DLL GetType()
static const char * _module = "ReportPolioVirusPopulation";// <<< Name of this file
static const std::string _report_name = "VirusPopulation.bin";


namespace Kernel
{
// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = { "POLIO_SIM", nullptr };// <<< Types of simulation the report is to be used with

report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new ReportPolioVirusPopulation()); // <<< Report to create
};

DllInterfaceHelper DLL_HELPER( _module, _sim_types, rif );

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// ------------------------------
// --- DLL Interface Methods
// ---
// --- The DTK will use these methods to establish communication with the DLL.
// ------------------------------

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

DTK_DLLEXPORT char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void __cdecl
GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char * __cdecl
GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT void __cdecl
GetReportInstantiator( Kernel::report_instantiator_function_t* pif )
{
    DLL_HELPER.GetReportInstantiator( pif );
}

#ifdef __cplusplus
}
#endif

// ----------------------------------------
// --- ReportPolioVirusPopulation Methods
// ----------------------------------------

void
ReportPolioVirusPopulation::Initialize( unsigned int nrmSize )
{
    int Run_Number = GET_CONFIG_INTEGER(EnvPtr->Config, "Run_Number");
    if( EnvPtr->MPI.Rank == 0)
    {
        std::string report_filename = boost::str(boost::format("%s/%s") % EnvPtr->OutputPath % _report_name);
        VirusPopulation.open(report_filename.c_str(), ios_base::out | ios_base::trunc | ios_base::binary);
    }
}

ReportPolioVirusPopulation::ReportPolioVirusPopulation()
{
}

void ReportPolioVirusPopulation::BeginTimestep()
{
}

void ReportPolioVirusPopulation::EndTimestep( float currentTime, float dt )
{
    Reduce_Internal();
}

void
ReportPolioVirusPopulation::LogNodeData( INodeContext * pNC )
{
    suids::suid_data_t suid = pNC->GetSuid().data;

    stringstream *node_stats = node_to_stats_map[suid];
    if( !node_stats )
        node_stats = node_to_stats_map[suid] = new stringstream (stringstream::in | stringstream::out | stringstream::binary);

    float tmp_float = pNC->GetTime().time;  node_stats->write((char*)(&tmp_float), sizeof(float));
    suids::suid_data_t tmp_suid = suid;     node_stats->write((char*)(&tmp_suid),  sizeof(suids::suid_data_t));
    tmp_float = pNC->GetStatPop();          node_stats->write((char*)(&tmp_float), sizeof(float));

    //const IPoolReportable *pr = pNC->GetPoolReportable();
    //tmp_float = pr->GetTotalContagion(TRANSMISSIONROUTE_ALL);
    //node_stats->write((char*)(&tmp_float), sizeof(float));
    //pr->ReportPopulations(node_stats);
}

void 
ReportPolioVirusPopulation::LogIndividualData( IIndividualHuman* individual )
{
}

void ReportPolioVirusPopulation::Finalize()
{
    if( EnvPtr->MPI.Rank == 0 )
        VirusPopulation.close();
}

void ReportPolioVirusPopulation::Reduce_Internal()
{
    stringstream tx_buffer (stringstream::in | stringstream::out | stringstream::binary);
    for( node_to_stringstream_map_t::iterator it = node_to_stats_map.begin(); it != node_to_stats_map.end(); /*it++*/)
    {
        suids::suid_data_t suid = it->first;
        stringstream *node_stats = it->second;
        it++;

        tx_buffer << node_stats->rdbuf();          // Write node stats
        node_stats->str(std::string());            // Clear for next iteration
    }
    // NOTE: Assuming that nothing nontrivial is remaining in the node_to_infections_map.  

    std::string tx_str(tx_buffer.str());
    std::string rx_str;
    
    LOG_DEBUG_F( "REDUCE:send_buffer (length %d): %s\n", tx_str.length(), tx_str.c_str() );

    EnvPtr->MPI.p_idm_mpi->GatherToRoot( tx_str, rx_str );

    if( rx_str.length() > 0 && VirusPopulation.good())
    {
        LOG_DEBUG_F( "REDUCE:receive_buffer (length %d): %s\n", rx_str.length(), rx_str.c_str());
        VirusPopulation.write(rx_str.c_str(), rx_str.size());
    }
}

void ReportPolioVirusPopulation::Reduce()
{
    Reduce_Internal();
}

std::string ReportPolioVirusPopulation::GetReportName() const
{
    return _report_name;
}
}