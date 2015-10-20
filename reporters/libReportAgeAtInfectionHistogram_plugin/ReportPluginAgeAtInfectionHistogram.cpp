/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportPluginAgeAtInfectionHistogram.h"
#include "Environment.h"
#include "Individual.h"
#include "FileSystem.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"
#include "FactorySupport.h" // for DTK_DLLEXPORT

using namespace std;
using namespace json;

static const char * _module = "ReportPluginAgeAtInfectionHistogram";

// You can put 0 or more valid Sim types into _sim_types but has to end with NULL
// Refer to DllLoader.h for current SIMTYPES_MAXNUM
// but we don't include DllLoader.h to avoid compiling redefinition errors.
static const char * _sim_types[] = {"GENERIC_SIM", "VECTOR_SIM", "MALARIA_SIM", "TB_SIM", "POLIO_SIM", NULL};

static const std::string _report_name = "AgeAtInfectionHistogramReport.json";

static const char * _infection_count_label = "BinnedInfectionCounts";
static const char * _age_bin_label = "AgeBinUpperEdgesInYears";
static const char * _reporting_interval_label = "ReportingIntervalInYears";

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

Kernel::report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new ReportPluginAgeAtInfectionHistogram()); 
};

Kernel::DllInterfaceHelper DLL_HELPER( _module, _sim_types, rif );
//
// This is the interface function from the DTK.
//
DTK_DLLEXPORT
char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void __cdecl
GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT void __cdecl
GetReportInstantiator( Kernel::report_instantiator_function_t* pif )
{
    DLL_HELPER.GetReportInstantiator( pif );
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

/////////////////////////
// Initialization methods
/////////////////////////


ReportPluginAgeAtInfectionHistogram::ReportPluginAgeAtInfectionHistogram()
{
    LOG_INFO( "ReportPluginAgeAtInfectionHistogram ctor\n" );
    report_name = _report_name;
    
    reporting_interval_in_years = 1;
    time_since_last_report = 0;
    int nbins = 10;
    for (int curr_bin = 0; curr_bin < nbins-1; curr_bin++)
    {
        age_bin_upper_edges_in_years.push_back((float)curr_bin/2.0f + 0.5);  //Initialize here however you like
        temp_binned_accumulated_counts.push_back(0.0f);
    }
    age_bin_upper_edges_in_years.push_back(10000); //put a catchall bin at the end
    temp_binned_accumulated_counts.push_back(0.0f);



}

/////////////////////////
// steady-state methods
/////////////////////////
void 
ReportPluginAgeAtInfectionHistogram::BeginTimestep()
{
}

void
ReportPluginAgeAtInfectionHistogram::EndTimestep( )
{
    time_since_last_report++;
    if (time_since_last_report >= (reporting_interval_in_years*DAYSPERYEAR))
    {
        LOG_DEBUG_F("Reporting Interval Reached\n");
        for(int curr_bin = 0; curr_bin < temp_binned_accumulated_counts.size(); curr_bin++) {
            LOG_DEBUG_F("Temp_binned_accumulated_counts[%d] = %f\n", curr_bin, temp_binned_accumulated_counts.at(curr_bin));
        }

        binned_accumulated_counts.push_back(temp_binned_accumulated_counts);
        std::fill(temp_binned_accumulated_counts.begin(), temp_binned_accumulated_counts.end(), 0.0f);
        time_since_last_report = 0;
    }
}

void
ReportPluginAgeAtInfectionHistogram::LogNodeData(
    Kernel::INodeContext * pNode
)
{
}

void 
ReportPluginAgeAtInfectionHistogram::LogIndividualData(
    Kernel::IndividualHuman * individual
)
{
    if (individual->GetNewInfectionState() == NewInfectionState::NewAndDetected || individual->GetNewInfectionState() == NewInfectionState::NewInfection)
    {
        //LOG_DEBUG_F("Individual's New infection state is %d, age is %d\n", (int)individual->GetNewInfectionState(), (int)individual->GetAge());
        int agebin_index = GetAgeBin(individual->GetAge()/DAYSPERYEAR);
        LOG_DEBUG_F("Adding MC weight %f to age bin %d\n", (float)individual->GetMonteCarloWeight(), (int)agebin_index);
        temp_binned_accumulated_counts.at(agebin_index) += (float)individual->GetMonteCarloWeight(); 
        //LOG_DEBUG_F("accumulated counts in bin %d is now %f\n", (int)agebin_index,  (float)temp_binned_accumulated_counts.at(agebin_index));
    }

}

int ReportPluginAgeAtInfectionHistogram::GetAgeBin(double age)
    {
        vector<float>::const_iterator it;
        it = std::lower_bound(age_bin_upper_edges_in_years.begin(), age_bin_upper_edges_in_years.end(), age);
        int agebin_idx = it - age_bin_upper_edges_in_years.begin();
        return agebin_idx;
}


// Just calling the base class but for demo purposes leaving in because I can imagine wanting to do this custom.
void
ReportPluginAgeAtInfectionHistogram::Finalize()
{
    LOG_INFO( "WriteData\n" );
    postProcessAccumulatedData();

    std::map<std::string, std::string> units_map;
    populateSummaryDataUnitsMap(units_map);

    time_t now = time(0);
#ifdef WIN32
    tm now2;
    localtime_s(&now2,&now);
    char timebuf[26];
    asctime_s(timebuf,26,&now2);
    std::string now3 = std::string(timebuf);
#else
    tm* now2 = localtime(&now);
    std::string now3 = std::string(asctime(now2));
#endif

    Element elementRoot = String();
    QuickBuilder qb(elementRoot);
    qb["Header"]["DateTime"] = String(now3.substr(0,now3.length()-1)); // have to remove trailing '\n'

    ProgDllVersion pv;
    ostringstream dtk_ver;
    dtk_ver << pv.getRevisionNumber() << " " << pv.getBranch() << " " << pv.getBuildDate();
    qb["Header"]["DTK_Version"] = String(dtk_ver.str());
    qb["Header"]["Report_Version"] = String("3");
    qb["Channels"]["ReportingInterval"]["Units"] = String("Years");
    qb["Channels"]["Age_Bin_Upper_Edges"]["Units"] = String("Years");
    qb["Channels"]["Accumulated_Binned_Infection_Counts"]["Units"] = String("Number of Infected");

    qb["Channels"]["ReportingInterval"]["Data"] = Number(reporting_interval_in_years);
    
    int curr_bin = 0;
    for (std::vector<float>::iterator it = age_bin_upper_edges_in_years.begin(); it!=age_bin_upper_edges_in_years.end(); ++it)
    {
        qb["Channels"]["Age_Bin_Upper_Edges"]["Data"][curr_bin] = Number(*it);
        curr_bin++;
    }

    int curr_interval = 0;
    for (std::vector< std::vector< float > >::iterator it1 = binned_accumulated_counts.begin(); it1 != binned_accumulated_counts.end(); ++it1)
    {
        curr_bin = 0;
        for (std::vector< float >::iterator it2 = (*it1).begin(); it2 != (*it1).end(); ++it2)
        {
            qb["Channels"]["Accumulated_Binned_Infection_Counts"]["Data"][curr_interval][curr_bin] = Number(*it2);
            curr_bin++;

        }
        curr_interval++;
    }
    

    // write to an internal buffer first... if we write directly to the network share, performance is slow
    // (presumably because it's doing a bunch of really small writes of all the JSON elements instead of one
    // big write)
    ostringstream oss;
    Writer::Write(elementRoot, oss);

    ofstream inset_chart_json;
    inset_chart_json.open( FileSystem::Concat( EnvPtr->OutputPath, report_name ).c_str() );

    if (inset_chart_json.is_open())
    {
        inset_chart_json << oss.str();
        inset_chart_json.close();
    }
    else
    {
        // ERROR ("Failed to open %s for writing inset chart data.\n", report_name.c_str());
        throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, report_name.c_str() );
    }
}

void 
ReportPluginAgeAtInfectionHistogram::populateSummaryDataUnitsMap(
    std::map<std::string, std::string> &units_map
)
{
    LOG_INFO( "populateSummaryDataUnitsMap\n" );
    units_map[_infection_count_label]                        = "Accumulated Binned Infection Counts";
    units_map[_age_bin_label]                                = "Age Bin Right Edges (years)";
    units_map[_reporting_interval_label]                     = "Reporting Interval (years)";
}

// not sure whether to leave this in custom demo subclass
void
ReportPluginAgeAtInfectionHistogram::postProcessAccumulatedData()
{
    LOG_DEBUG( "getSummaryDataCustomProcessed\n" );
}

void
ReportPluginAgeAtInfectionHistogram::Reduce()
{
    LOG_INFO( "Reduce\n" );
    
}
