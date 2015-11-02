/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include <functional>
#include <numeric>
#include <map>
#include "BoostLibWrapper.h"
// not in boost wrapper???
#include <boost/math/special_functions/fpclassify.hpp>

#include "SpatialReportPolio.h"
#include "NodePolio.h"
#include "Sugar.h"
#include "Environment.h"
#include "Exceptions.h"
#include "Individual.h"
#include "SimulationConfig.h"
#include "ProgVersion.h"

using namespace std;

static const char * _module = "SpatialReportPolio";

namespace Kernel {

GET_SCHEMA_STATIC_WRAPPER_IMPL(SpatialReportPolio,SpatialReportPolio)

/////////////////////////
// Initialization methods
/////////////////////////
IReport*
SpatialReportPolio::CreateReport()
{
    return new SpatialReportPolio();
}

SpatialReportPolio::SpatialReportPolio()
: SpatialReport()
, age_infection_info(           "Age_Infection",                "years")
, total_prevalence_info(        "Total_Prevalence",             "infected fraction")
, new_paralytic_cases_info(     "New_Paralytic_Cases",          "")
, wpv1_prevalence_info(         "WPV1_Prevalence",              "infected fraction")
, wpv2_prevalence_info(         "WPV2_Prevalence",              "infected fraction")
, wpv3_prevalence_info(         "WPV3_Prevalence",              "infected fraction")
, vrpv1_prevalence_info(        "VDPV1_Prevalence",             "infected fraction")
, vrpv2_prevalence_info(        "VDPV2_Prevalence",             "infected fraction")
, vrpv3_prevalence_info(        "VDPV3_Prevalence",             "infected fraction")
, node_infection_total(0)
{
    new_paralytic_cases = 0.0f;
}

void SpatialReportPolio::Initialize( unsigned int nrmSize )
{
    SpatialReport::Initialize( nrmSize );

    if( age_infection_info.enabled && !total_prevalence_info.enabled )
    {
        LOG_WARN("Age_Infection requires that Total_Prevalence be enabled.  Enabling Total_Prevalence.");
        total_prevalence_info.enabled = true ;
        channelDataMap.IncreaseChannelLength( total_prevalence_info.name, _nrmSize );
    }
}

void SpatialReportPolio::populateChannelInfos(tChanInfoMap &channel_infos)
{
    SpatialReport::populateChannelInfos(channel_infos);

    channel_infos[ age_infection_info.name ] = &age_infection_info;
    channel_infos[ total_prevalence_info.name ] = &total_prevalence_info;
    channel_infos[ new_paralytic_cases_info.name ] = &new_paralytic_cases_info;
    channel_infos[ wpv1_prevalence_info.name ] = &wpv1_prevalence_info;
    channel_infos[ wpv2_prevalence_info.name ] = &wpv2_prevalence_info;
    channel_infos[ wpv3_prevalence_info.name ] = &wpv3_prevalence_info;
    channel_infos[ vrpv1_prevalence_info.name ] = &vrpv1_prevalence_info;
    channel_infos[ vrpv2_prevalence_info.name ] = &vrpv2_prevalence_info;
    channel_infos[ vrpv3_prevalence_info.name ] = &vrpv3_prevalence_info;
}


void SpatialReportPolio::LogIndividualData( Kernel::IndividualHuman * individual )
{
    SpatialReport::LogIndividualData(individual);

    float monte_carlo_weight = (float)individual->GetMonteCarloWeight();

    NewInfectionState::_enum nis = individual->GetNewInfectionState();

    auto infectionsOfStrains = dynamic_cast<const IndividualHumanPolio*>(individual)->GetSusceptibilityReporting()->GetInfectionStrains();

    if(nis == NewInfectionState::NewlyDetected || nis == NewInfectionState::NewAndDetected)
        new_paralytic_cases += monte_carlo_weight;

    auto nodeid = individual->GetParentSuid().data;
    if(wpv1_prevalence_info.enabled)
        Accumulate(wpv1_prevalence_info.name, nodeid, infectionsOfStrains[0]);

    if(wpv2_prevalence_info.enabled)
        Accumulate(wpv2_prevalence_info.name, nodeid, infectionsOfStrains[1]);

    if(wpv3_prevalence_info.enabled)
        Accumulate(wpv3_prevalence_info.name, nodeid, infectionsOfStrains[2]);

    if(vrpv1_prevalence_info.enabled)
        Accumulate(vrpv1_prevalence_info.name, nodeid, infectionsOfStrains[3]);

    if(vrpv2_prevalence_info.enabled)
        Accumulate(vrpv2_prevalence_info.name, nodeid, infectionsOfStrains[4]);

    if(vrpv3_prevalence_info.enabled)
        Accumulate(vrpv3_prevalence_info.name, nodeid, infectionsOfStrains[5]);

    node_infection_total = std::accumulate( infectionsOfStrains, infectionsOfStrains + 6, 0 );

    if(total_prevalence_info.enabled)
        Accumulate(total_prevalence_info.name, nodeid, node_infection_total);
}

void
SpatialReportPolio::LogNodeData(
    Kernel::INodeContext * pNC
)
{
    SpatialReport::LogNodeData(pNC);

    int nodeid = pNC->GetExternalID();

    const Kernel::INodePolio * pPolioNode = nullptr;
    if( pNC->QueryInterface( GET_IID( Kernel::INodePolio), (void**) &pPolioNode ) != Kernel::s_OK )
    {
        throw Kernel::QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodePolio", "INodeContext" );
    }

    if(age_infection_info.enabled)
        Accumulate(age_infection_info.name, nodeid, pPolioNode->GetMeanAgeInfection() * node_infection_total);

    if(new_paralytic_cases_info.enabled)
    {
        Accumulate(new_paralytic_cases_info.name, nodeid, new_paralytic_cases);
        new_paralytic_cases = 0.0f;
    }

    node_infection_total = 0;
}

void
SpatialReportPolio::postProcessAccumulatedData()
{
    SpatialReport::postProcessAccumulatedData();

    if( age_infection_info.enabled && total_prevalence_info.enabled )
    {
        // pass through normalization
        // order matters, since we're changing channels in place (not like old way)
        normalizeChannel(age_infection_info.name, total_prevalence_info.name);
    }

    if( total_prevalence_info.enabled )
        normalizeChannel(total_prevalence_info.name, population_info.name);

    if( wpv1_prevalence_info.enabled )
        normalizeChannel(wpv1_prevalence_info.name, population_info.name);

    if( wpv2_prevalence_info.enabled )
        normalizeChannel(wpv2_prevalence_info.name, population_info.name);

    if( wpv3_prevalence_info.enabled )
        normalizeChannel(wpv3_prevalence_info.name, population_info.name);

    if( vrpv1_prevalence_info.enabled )
        normalizeChannel(vrpv1_prevalence_info.name, population_info.name);

    if( vrpv2_prevalence_info.enabled )
        normalizeChannel(vrpv2_prevalence_info.name, population_info.name);

    if( vrpv3_prevalence_info.enabled )
        normalizeChannel(vrpv3_prevalence_info.name, population_info.name);
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(SpatialReport)
template<class Archive>
void serialize(Archive &ar, SpatialReportPolio& report, const unsigned int v)
{
    boost::serialization::void_cast_register<SpatialReportPolio,IReport>();
    ar & report.timesteps_reduced;
    ar & report.channelDataMap;
    ar & report._nrmSize;
}
#endif

}

#endif // ENABLE_POLIO
