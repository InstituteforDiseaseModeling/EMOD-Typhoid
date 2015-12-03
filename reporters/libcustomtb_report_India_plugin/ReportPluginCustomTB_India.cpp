/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ReportPluginCustomTB_India.h"

#include <boost/assign/list_of.hpp>
#include <map>
#include <string>
#include "mat.h"

#include "BoostLibWrapper.h"
#include "Environment.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "IndividualTB.h"
#include "Sugar.h"

#include "DllDefs.h"
#include "ProgVersion.h"

// Module name for logging
static const char * _module = "ReportPluginCustomTBIndia"; 

// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr
// Refer to DllLoader.h for current SIMTYPES_MAXNUM
// but we don't include DllLoader.h to avoid compiling redefinition errors.
static const char * _sim_types[] = {"TB_SIM", nullptr};

// Output file name
static const std::string _report_name = "CustomTBReportIndia.mat";


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
    while (_sim_types[i] != NULL && i < SIMTYPES_MAXNUM )
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
    CustomTBReportIndiaMATLAB* report =  new CustomTBReportIndiaMATLAB();
    return report;
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



CustomTBReportIndiaMATLAB::CustomTBReportIndiaMATLAB() :
    new_active_smear_pos(0.0f),
    disease_deaths_smear_pos(0.0f),
    Report()
{
    LOG_DEBUG( "CTOR\n" );
}

CustomTBReportIndiaMATLAB::~CustomTBReportIndiaMATLAB()
{
    LOG_DEBUG( "DTOR\n" );

}

void CustomTBReportIndiaMATLAB::Initialize( unsigned int nrmSize )
{
    LOG_DEBUG( "Initialize\n" );

    report_name = _report_name;
}



void CustomTBReportIndiaMATLAB::EndTimestep( float currentTime, float dt )
{
    Accumulate( "New_active_smear_pos", new_active_smear_pos );
    new_active_smear_pos = 0.0f;
    Accumulate( "Disease_deaths_smear_pos", disease_deaths_smear_pos );
    disease_deaths_smear_pos = 0.0f;
    Report::EndTimestep( currentTime, dt );
}

void  CustomTBReportIndiaMATLAB::LogIndividualData( Kernel::IIndividualHuman* individual )
{
    LOG_DEBUG( "LogIndividualData\n" );

    // Get individual weight and sub-channel binned variables
    float mc_weight    = (float)individual->GetMonteCarloWeight();
    const Kernel::IndividualHumanTB* individual_tb = static_cast<const Kernel::IndividualHumanTB*>(individual); //save this pointer to individual_tb for future outputs from individual_tb

    if (individual->GetNewInfectionState() == NewInfectionState::NewlyActive && individual_tb->IsSmearPositive() == true)
    {
        new_active_smear_pos += mc_weight;
    }

    // Accumulate appropriate channel/sub-channel for this individual
    if ( individual->GetStateChange() == HumanStateChange::KilledByInfection && individual_tb->IsSmearPositive() == true)
    {
        disease_deaths_smear_pos += mc_weight;
    }

    /*
    if (individual->IsInfected())
    {
        switch( individual->GetNewInfectionState() )
        {
        case NewInfectionState::NewInfection:
            new_infections_bins += mc_weight;
            break;
        }
        
        const Kernel::IndividualHumanTB* individual_tb = static_cast<const Kernel::IndividualHumanTB*>(individual); //save this pointer to individual_tb for future outputs from individual_tb
    }
    */
}

void CustomTBReportIndiaMATLAB::Finalize()
{
    LOG_INFO( "Finalize\n" );

    // open MATLAB output file
    MATFile *pmat;
    LOG_INFO_F( "Creating file: %s\n", report_name.c_str() );
    pmat = matOpen( FileSystem::Concat( EnvPtr->OutputPath, report_name ).c_str(), "w");

    // error handling
    if (pmat == NULL) 
    {
        throw Kernel::FileNotFoundException( __FILE__, __LINE__, __FUNCTION__, report_name.c_str() );
    }

    // get date/time
    time_t now = time(0);
    tm now2;
    localtime_s(&now2,&now);
    char timebuf[26];
    asctime_s(timebuf,26,&now2);
    std::string now3 = std::string(timebuf);
    std::string now4 = now3.substr(0,now3.length()-1); // have to remove trailing '\n'

    // get DTK version
    ProgDllVersion pv;
    ostringstream dtk_ver;
    dtk_ver << pv.getRevisionNumber() << " " << pv.getBranch() << " " << pv.getBuildDate();

    // get timesteps
    int ntimesteps = channelDataMap.begin()->second.size();

    // get # of channels
    int nchannels  = channelDataMap.size();

    // create 1x1 struct called 'Header
    mwSize dim1by1[2] = {1, 1};
    const char *header_field_names[] = { "DateTime", "DTK_Version", "Timesteps", "Channels" };
    mxArray *header = mxCreateStructArray(2, dim1by1, 4, header_field_names);

     // set fields in "Header"
    mxSetField(header, 0, "DateTime",    mxCreateString( now4.c_str() ) );
    mxSetField(header, 0, "DTK_Version", mxCreateString( dtk_ver.str().c_str() ) );
    mxSetField(header, 0, "Timesteps",   mxCreateDoubleScalar( ntimesteps ) );
    mxSetField(header, 0, "Channels",    mxCreateDoubleScalar( nchannels ) );

    // create cell array for "ChannelTitles" of dimension 1 x nchannels
    mwSize dim1byN[2] = {1, nchannels};
    mxArray *titles = mxCreateCellArray(2, dim1byN);

    // ... and another cell array for "ChannelUnits"
    mxArray *units = mxCreateCellArray(2, dim1byN);
    std::map<std::string, std::string> units_map;
    populateSummaryDataUnitsMap(units_map);

    // ...and one more cell array for "ChannelData"
    mxArray *data = mxCreateCellArray(2, dim1byN);

    // loop over all channels
    int i=0;
    mwSize dim1byT[2] = {1, ntimesteps};
    std::vector<mxArray*> p_data(nchannels);
    foreach(channel_data_map_t::value_type pair, channelDataMap)
    {
        std::string name = pair.first;
        const Report::channel_data_t *channel_data = &(pair.second);

        // fill cell arrays of titles and units
        mxSetCell( titles, i, mxCreateString( name.c_str() ) );
        mxSetCell( units,  i, mxCreateString( units_map[name].c_str() ) );

        // using arrays instead of cell arrays for "ChannelData" 
        // results in significantly reduced output file size
        p_data[i] = mxCreateDoubleMatrix(1, channel_data->size(), mxREAL);
        std::copy( channel_data->begin(), channel_data->end(), mxGetPr(p_data[i]) );

        // fill cell array of data channels
        mxSetCell( data, i, p_data[i] );

        i++;
    }

    // create top-level 1x1 struct for "InsetCharts"
    const char *chart_field_names[] = { "Header", "ChannelTitles", "ChannelUnits", "ChannelData" };
    mxArray *charts = mxCreateStructArray(2, dim1by1, 4, chart_field_names);

    // set fields in "InsetCharts"
    mxSetField( charts, 0, "Header",        header );
    mxSetField( charts, 0, "ChannelTitles", titles );
    mxSetField( charts, 0, "ChannelUnits",  units  );
    mxSetField( charts, 0, "ChannelData",   data   );

    // put MATLAB top-level struct array into file
    int status = matPutVariable(pmat, "InsetCharts", charts);
    if (status != 0) 
    {
        throw Kernel::DetailedException(__FILE__, __LINE__, __FUNCTION__);
    }

    // close MATLAB file
    if (matClose(pmat) != 0)
    {
        throw Kernel::DetailedException( __FILE__, __LINE__, __FUNCTION__);
    }
}
