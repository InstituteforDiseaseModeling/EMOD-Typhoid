/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "StdAfx.h"

#include <fstream>

#include "BoostLibWrapper.h"
#include "ReportPolioVirusPopulation.h"
#include "Environment.h"
#include "Node.h"
#include "IndividualPolio.h"
#include "InfectionPolio.h"
#include "Log.h"

#include "ProgVersion.h"
#include "DllDefs.h"

#pragma warning(disable : 4996)

using namespace std;
using namespace json;

static const char * _module = "ReportPolioVirusPopulation";

static const std::string _report_name = "VirusPopulation.bin";

using namespace Kernel;

// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr
// Refer to DllLoader.h for current SIMTYPES_MAXNUM
// but we don't include DllLoader.h to avoid compiling redefinition errors.
static const char * _sim_types[] = {"POLIO_SIM", nullptr};

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

//
// This is the interface function from the DTK.
//
DTK_DLLEXPORT
char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    Environment::setInstance(const_cast<Environment*>(pEnv));
    ProgDllVersion pv;
    LOG_INFO_F("GetVersion called with ver=%s\n", pv.getVersion());
    if (sVer) strcpy(sVer, pv.getVersion());
    return sVer;
}


DTK_DLLEXPORT void __cdecl
GetSupportedSimTypes(char* simTypes[])
{
    int i=0;
    while (i < SIMTYPES_MAXNUM && _sim_types[i] != NULL)
    {
        // allocation will be freed by the caller
        simTypes[i] = new char[strlen(_sim_types[i]) + 1];
        strcpy(simTypes[i], _sim_types[i]);
        i++;
    }
    simTypes[i] = NULL;
}

DTK_DLLEXPORT IReport* __cdecl
CreateReport()
{
    std::ostringstream oss;
    oss << "CreateReport called for " << _module << std::endl;
    LOG_INFO( oss.str().c_str() );
    return new ReportPolioVirusPopulation();
}

DTK_DLLEXPORT
const char *
__cdecl
GetType()
{
    std::ostringstream oss;
    oss << "GetType called for " << _module << std::endl;
    LOG_INFO( oss.str().c_str() );
    return _module;
}

DTK_DLLEXPORT void __cdecl
InitReportEnvironment(
    const Environment * pEnv
)
{
    Environment::setInstance(const_cast<Environment*>(pEnv));
}

#ifdef __cplusplus
}
#endif


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

    float tmp_float = pNode->GetTime();     node_stats->write((char*)(&tmp_float), sizeof(float));
    suids::suid_data_t tmp_suid = suid;     node_stats->write((char*)(&tmp_suid),  sizeof(suids::suid_data_t));
    tmp_float = pNode->GetStatPop();        node_stats->write((char*)(&tmp_float), sizeof(float));

    const IPoolReportable *pr = pNode->GetPoolReportable();

    tmp_float = pr->GetTotalContagion(TRANSMISSIONROUTE_ALL);
    node_stats->write((char*)(&tmp_float), sizeof(float));

    pr->ReportPopulations(node_stats);
}

void 
ReportPolioVirusPopulation::LogIndividualData( IndividualHuman * individual )
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

    boost::mpi::reduce(
        *(EnvPtr->MPI.World), // communicator
        tx_str, // local value to be combined with local values of every other process
        rx_str, // local value to be combined with local values of every other process
        std::plus< std::string >(),
        0); // root rank, 0 not surprisingly

    if( rx_str.length() > 0 && VirusPopulation.good())
    {
        LOG_DEBUG_F( "REDUCE:receive_buffer (length %d): %s\n", rx_str.length(), rx_str.c_str());
        VirusPopulation.write(rx_str.c_str(), rx_str.size());
    }
}

void ReportPolioVirusPopulation::Reduce()
{

}

std::string ReportPolioVirusPopulation::GetReportName() const
{
    return _report_name;
}
