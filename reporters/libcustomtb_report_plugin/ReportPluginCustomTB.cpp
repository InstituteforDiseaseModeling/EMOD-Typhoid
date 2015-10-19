/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ReportPluginCustomTB.h"

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
static const char * _module = "ReportPluginCustomTB"; 

// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr
// Refer to DllLoader.h for current SIMTYPES_MAXNUM
// but we don't include DllLoader.h to avoid compiling redefinition errors.
static const char * _sim_types[] = {"TB_SIM", nullptr};

// Output file name
static const std::string _report_name = "CustomTBReport.mat";

// Static variables that define sub-channel binning
static const int   _num_age_bins = 4;
static const float _age_bin_upper_values[]   = {1825.0, 3650.0, 7300.0, 300000 };
static const char* _age_bin_friendly_names[] = {  "<5", "5-10", "10-20", ">20" };

static const char* _axis_labels[]       = { "Age", "FirstLineCombo", "Vaccine" };
static const int   _num_bins_per_axis[] = { _num_age_bins, 2, 2 };

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
    CustomTBReportMATLAB* report =  new CustomTBReportMATLAB();
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

CustomTBReportMATLAB::CustomTBReportMATLAB() : BinnedReport()
{
    LOG_DEBUG( "CTOR\n" );
}

CustomTBReportMATLAB::~CustomTBReportMATLAB()
{
    LOG_DEBUG( "DTOR\n" );

    // free memory allocated to channel-value accumulation
    delete disease_deaths_bins;
    delete new_infections_bins;
}

void CustomTBReportMATLAB::Initialize( unsigned int nrmSize )
{
    LOG_DEBUG( "Initialize\n" );

    report_name = _report_name;

    static_assert(_countof(_axis_labels) == _countof(_num_bins_per_axis), "Number of axis-labels must match number of axis bin-counts");

    axis_labels = std::vector<std::string>(_axis_labels, _axis_labels + (sizeof(_axis_labels) / sizeof(char*)));
    num_bins_per_axis = std::vector<int>(_num_bins_per_axis, _num_bins_per_axis + (sizeof(_num_bins_per_axis) / sizeof(int)));

    num_axes = _countof(_axis_labels);

    num_total_bins = 1;
    foreach(int i, num_bins_per_axis)
        num_total_bins *= i;

    static_assert( _countof(_age_bin_upper_values)   == _num_age_bins, "Number of age-bins must match specified size" );
    static_assert( _countof(_age_bin_friendly_names) == _num_age_bins, "Number of age-bins friendly-names must match specified size" );

    // push back age binning
    values_per_axis.push_back( std::vector<float>(_age_bin_upper_values, _age_bin_upper_values + (sizeof(_age_bin_upper_values) / sizeof(int))));
    friendly_names_per_axis.push_back( std::vector<std::string>(_age_bin_friendly_names, _age_bin_friendly_names + (sizeof(_age_bin_friendly_names) / sizeof(char*))));
    
    // push back bednet binning
    values_per_axis.push_back( std::vector<float>( boost::assign::list_of(0.0f)(1.0f).convert_to_container<std::vector<float>>() ) );
    friendly_names_per_axis.push_back( std::vector<std::string>( boost::assign::list_of("No FirstLineCombo")("Received FirstLineCombo").convert_to_container<std::vector<std::string>>() ) );

    // push back vaccine binning
    values_per_axis.push_back( std::vector<float>( boost::assign::list_of(0.0f)(1.0f).convert_to_container<std::vector<float>>() ) );
    friendly_names_per_axis.push_back( std::vector<std::string>( boost::assign::list_of("No Vaccine")("Received Vaccine").convert_to_container<std::vector<std::string>>() ) );

    initChannelBins();
}

void CustomTBReportMATLAB::initChannelBins()
{
    LOG_DEBUG( "initChannelBins\n" );

    BinnedReport::initChannelBins();

    // allocate accumulated channel value arrays
    disease_deaths_bins = new float[num_total_bins];
    new_infections_bins = new float[num_total_bins];

    clearChannelsBins();
}

void CustomTBReportMATLAB::clearChannelsBins()
{
    memset(disease_deaths_bins, 0, num_total_bins * sizeof(float));
    memset(new_infections_bins, 0, num_total_bins * sizeof(float));
}

void CustomTBReportMATLAB::EndTimestep( float currentTime, float dt )
{
    Accumulate( "TB Deaths", disease_deaths_bins );
    Accumulate( "New TB Infections", new_infections_bins );

    BinnedReport::EndTimestep( currentTime, dt );

    clearChannelsBins();
}

void  CustomTBReportMATLAB::LogIndividualData( Kernel::IndividualHuman * individual )
{
    LOG_DEBUG( "LogIndividualData\n" );

    // Get individual weight and sub-channel binned variables
    float mc_weight    = (float)individual->GetMonteCarloWeight();
    float age          = (float)individual->GetAge();

    // N.B. unless they are discarded at end of box duration (or after sufficient exponential efficacy decay)
    // the interventions might still be a useless one!!
    Kernel::IIndividualHumanInterventionsContext * intervs = individual->GetInterventionsContext();
    bool receivedVaccine = intervs->GetInterventionsByType( "class Kernel::SimpleVaccine" ).size() > 0;
    bool receivedBednet  = intervs->GetInterventionsByType( "class Kernel::SimpleDiagnostic"  ).size() > 0;

    // Calculate sub-channel
    int agebin   = std::lower_bound( values_per_axis[0].begin(), values_per_axis[0].end(), age ) - values_per_axis[0].begin();
    int bin_index = ( num_bins_per_axis[0] * num_bins_per_axis[1] * receivedVaccine ) + ( num_bins_per_axis[0] * receivedBednet ) + agebin;

    // Accumulate appropriate channel/sub-channel for this individual
    // Recall that the unmodified values of the "_bins" arrays are memset to zero previously
    population_bins[bin_index] += mc_weight;

    if ( individual->GetStateChange() == HumanStateChange::KilledByInfection )
    {
        disease_deaths_bins[bin_index] += mc_weight;
    }

    if (individual->IsInfected())
    {
        switch( individual->GetNewInfectionState() )
        {
        case NewInfectionState::NewInfection:
            new_infections_bins[bin_index] += mc_weight;
            break;
        }
        
        const Kernel::IndividualHumanTB* individual_tb = static_cast<const Kernel::IndividualHumanTB*>(individual); //save this pointer to individual_tb for future outputs from individual_tb
    }
}

void CustomTBReportMATLAB::Finalize()
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

    // create 1x1 struct called 'Header'
    const char *header_field_names[] = { "DateTime", "DTK_Version", "Timesteps", "Channels", "Subchannel_Metadata" };
    mxArray *header = mxCreateStructMatrix(1, 1, 5, header_field_names);

    // create 1x1 struct called 'Subchannel_Metadata'
    
    const char *subchannel_fields[] = { "AxisLabels", "NumBinsPerAxis"/*, "ValuesPerAxis", "MeaningPerAxis"*/ };
    mxArray *subchannels = mxCreateStructMatrix(1, 1, /*4*/2, subchannel_fields);

    // set fields in "Subchannel_Metadata"
    mxArray *nbins = mxCreateDoubleMatrix(1, num_bins_per_axis.size(), mxREAL);
    std::copy( num_bins_per_axis.begin(), num_bins_per_axis.end(), mxGetPr(nbins) );
    mxSetField(subchannels, 0, "NumBinsPerAxis", nbins );

    mxArray *labels = mxCreateCellMatrix( 1, axis_labels.size() );
    int i=0;
    foreach( std::string label, axis_labels )
    {
        mxSetCell( labels, i, mxCreateString( label.c_str() ) );
        i++;
    }
    mxSetField(subchannels, 0, "AxisLabels", labels);

    // TODO: FINISH THIS PART (probably with cell matrices)
    //mxArray *axisvals = mxCreateStructMatrix(1, 1, num_axes, &label_strings[0]);
    //mxSetField(axisvals, 0, axis_labels[0].c_str(), mxCreateDoubleMatrix(values_per_axis[0]...));
    //mxSetField(subchannels, 0, "ValuesPerAxis", axisvals);

    //mxArray *axismeanings = mxCreateStructMatrix(1, 1, num_axes, &label_strings[0]);
    //mxSetField(axismeanings, 0, axis_labels[0].c_str(), mxCreateCharMatrixFromStrings(friendly_names_per_axis...));
    //mxSetField(subchannels, 0, "MeaningPerAxis", axismeanings);

    // set fields in "Header"
    mxSetField(header, 0, "DateTime",    mxCreateString( now4.c_str() ) );
    mxSetField(header, 0, "DTK_Version", mxCreateString( dtk_ver.str().c_str() ) );
    mxSetField(header, 0, "Timesteps",   mxCreateDoubleScalar( ntimesteps ) );
    mxSetField(header, 0, "Channels",    mxCreateDoubleScalar( nchannels ) );
    mxSetField(header, 0, "Subchannel_Metadata", subchannels);

    // create cell array for "ChannelTitles" of dimension 1 x nchannels
    mxArray *titles = mxCreateCellMatrix(1, nchannels);

    // ...and another cell array for "ChannelData"
    mxArray *data = mxCreateCellMatrix(1, nchannels);

    // loop over all channels
    i=0;
    std::vector<mxArray*> p_data(nchannels);
    foreach(channel_data_map_t::value_type pair, channelDataMap)
    {
        std::string name = pair.first;
       // old does not work const Report::channel_data_t *channel_data = &(pair.second);
        const channel_data_t *channel_data = &(pair.second);
        // fill cell arrays of titles
        mxSetCell( titles, i, mxCreateString( name.c_str() ) );

        // using arrays instead of cell arrays for "ChannelData" 
        // results in significantly reduced output file size
        p_data[i] = mxCreateDoubleMatrix(1, channel_data->size(), mxREAL);
        std::copy( channel_data->begin(), channel_data->end(), mxGetPr(p_data[i]) );

        // TODO: if we have time, we can reshape here rather than in MATLAB 
        // let's  match [10 age bins] [2 bednet bins] [2 vaccine bins] [N time step bins]
        // from the single channel_data vector of 40 * N bins
        // to a 10x2x2xN matrix with mxCreateNumericArray ??
        //p_data[i] = mxCreateNumericArray(4, subBinDims, mxSINGLE_CLASS, mxREAL);
        // should actually work with a memcpy done carefully...

        // copy the vector<float> to the mxArray
        // std::copy( channel_data->begin(), channel_data->end(), mxGetPr(p_data[i]) );
        // (TODO: does this work for arbitrary subBinDims?)
        // meh, never mind.  matrix resizing is easy enough in MATLAB, so let's just do it there...
        // e.g. pop = reshape(InsetCharts.ChannelData{9}, 10, 2, 2, []); % sub-binning
        //      plot(squeeze(pop(1,1,1,:))); % plot population of under-1-year-olds w/o interventions vs. time step
        // e.g. pp  = reshape(InsetCharts.ChannelData{8}, 10, 2, 2, []);
        //      plot((squeeze(sum(sum(pp,2),3))./squeeze(sum(sum(pop,2),3)))'); % prevalence by age bin (averaged over all intervention statuses)

        // fill cell array of data channels
        mxSetCell( data, i, p_data[i] );

        i++;
    }

    // create top-level 1x1 struct for "InsetCharts"
    const char *chart_field_names[] = { "Header", "ChannelTitles", "ChannelData" };
    mxArray *charts = mxCreateStructMatrix(1, 1, 3, chart_field_names);

    // set fields in "InsetCharts"
    mxSetField( charts, 0, "Header",        header );
    mxSetField( charts, 0, "ChannelTitles", titles );
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
        throw Kernel::DetailedException(__FILE__, __LINE__, __FUNCTION__);
    }

    // clean up
    // probably some of the mxArrays too...
    //for ( size_t i=0; i<label_strings.size(); i++ )
    //    delete [] label_strings[i];
}
