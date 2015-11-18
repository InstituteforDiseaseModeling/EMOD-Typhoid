/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include <numeric> // for std::accumulate
#include "ReportPolio.h" // for base class
#include "NodePolio.h" // for base class
#include "Debug.h" // for base class

static const char * _module = "ReportPolio";


namespace Kernel {

static const char * _aoi_label = "Age Of Infection";
static const string _wpv1_prev_label("WPV1 Prevalence");
static const string _wpv2_prev_label("WPV2 Prevalence");
static const string _wpv3_prev_label("WPV3 Prevalence");
static const string _vdpv1_prev_label("VDPV1 Prevalence");
static const string _vdpv2_prev_label("VDPV2 Prevalence");
static const string _vdpv3_prev_label("VDPV3 Prevalence");
static const string _tot_prev_label("Total Prevalence");
static const string _wpv1_log_prev_label("WPV1 Log Prevalence");
static const string _wpv2_log_prev_label("WPV2 Log Prevalence");
static const string _wpv3_log_prev_label("WPV3 Log Prevalence");
static const string _vdpv1_log_prev_label("VDPV1 Log Prevalence");
static const string _vdpv2_log_prev_label("VDPV2 Log Prevalence");
static const string _vdpv3_log_prev_label("VDPV3 Log Prevalence");
static const string _log_tot_prev_label("Log Total Prevalence");
static const string _log_prev_label("Log Prevalence");
static const string _wpv1_cum_paral_cases_label("Cumulative Paralytic Cases WPV1");
static const string _wpv2_cum_paral_cases_label("Cumulative Paralytic Cases WPV2");
static const string _wpv3_cum_paral_cases_label("Cumulative Paralytic Cases WPV3");
static const string _vdpv1_cum_paral_cases_label("Cumulative Paralytic Cases VDPV1");
static const string _vdpv2_cum_paral_cases_label("Cumulative Paralytic Cases VDPV2");
static const string _vdpv3_cum_paral_cases_label("Cumulative Paralytic Cases VDPV3");
static const string _cum_paral_cases_label("Cumulative Paralytic Cases");
static const string _cum_suscept_infections_label("Cumulative Disease Susceptible Infections");
static const string _immunity_label("Mean Immunity At New Infection");
static const string _seroprevalence_label_1("Seroprevalence (P1)");
static const string _seroprevalence_label_2("Seroprevalence (P2)");
static const string _seroprevalence_label_3("Seroprevalence (P3)");
static const string _genetic_entropy_label("Genetic Entropy");


GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportPolio,ReportPolio)


ReportPolio::ReportPolio()
    : total_cumulative_paralytic_cases(0.0f)
    , cumulativeDiseaseSusceptibleInfections(0.0f)
    , infectionStrainIdCounts()
    , immunityAtInfection(0.0f)
    , total_infections(0)
    , DiseaseSusceptibleReportPoliovirusType(PolioVirusTypes::WPV1)
{
    ZERO_ARRAY(cumulative_paralytic_cases);
    ZERO_ARRAY(seroprevalence);
}

bool ReportPolio::Configure( const Configuration * inputJson )
{
    initConfig( "Disease_Susceptible_Report_Poliovirus_Type",
                DiseaseSusceptibleReportPoliovirusType,
                inputJson,
                MetadataDescriptor::Enum( "Disease_Susceptible_Report_Poliovirus_Type",
                                           Disease_Susceptible_Report_Poliovirus_Type_DESC_TEXT,
                                           MDD_ENUM_ARGS(PolioVirusTypes)
                                        )
                );

    bool ret = JsonConfigurable::Configure( inputJson );
    return ret ;
}

void ReportPolio::EndTimestep( float currentTime, float dt )
{
    ReportEnvironmental::EndTimestep( currentTime, dt );

    Accumulate( _wpv1_cum_paral_cases_label, cumulative_paralytic_cases[PolioVirusTypes::WPV1] );
    Accumulate( _wpv2_cum_paral_cases_label, cumulative_paralytic_cases[PolioVirusTypes::WPV2] );
    Accumulate( _wpv3_cum_paral_cases_label, cumulative_paralytic_cases[PolioVirusTypes::WPV3] );
    Accumulate( _vdpv1_cum_paral_cases_label, cumulative_paralytic_cases[PolioVirusTypes::VRPV1] );
    Accumulate( _vdpv2_cum_paral_cases_label, cumulative_paralytic_cases[PolioVirusTypes::VRPV2] );
    Accumulate( _vdpv3_cum_paral_cases_label, cumulative_paralytic_cases[PolioVirusTypes::VRPV3] );
    Accumulate( _cum_paral_cases_label, total_cumulative_paralytic_cases );
    Accumulate( _cum_suscept_infections_label,  cumulativeDiseaseSusceptibleInfections );
    // Record and clear 'mean immunity upon infection' channel data
    Accumulate( _immunity_label, immunityAtInfection );
    immunityAtInfection = 0;
    // Record and clear 'seroprevalence (for each serotype)' channel data
    Accumulate( _seroprevalence_label_1, seroprevalence[PolioSerotypes::PV1] );
    Accumulate( _seroprevalence_label_2, seroprevalence[PolioSerotypes::PV2] );
    Accumulate( _seroprevalence_label_3, seroprevalence[PolioSerotypes::PV3] );
    seroprevalence[PolioSerotypes::PV1] = 0;
    seroprevalence[PolioSerotypes::PV2] = 0;
    seroprevalence[PolioSerotypes::PV3] = 0;

    // Calculate and store genetic entropy channel data.
    float entropy = 0;
    unsigned int totalInfections = 0;
    tStrainIdToOccurrencesMap probs = infectionStrainIdCounts;
    // We have collected occurrences of each strain id in infectionStrainIdCounts, 
    // where the index/key is the strain id, and the value is the number of occurrences.
    for( unsigned int idx = 0; idx<infectionStrainIdCounts.size(); ++idx )
    {
        totalInfections += infectionStrainIdCounts[ idx ];
        infectionStrainIdCounts[ idx ] = 0;
    }

    // Now calculate the entropy using Boltzmann's equation: -1 * SUM over N of { P(n) * log P(n) }
    // where N is the number of states and P(n) is the probability of that state based
    // on actual recorded occurrences.
    if( totalInfections )
    {
        for( unsigned int idx = 0; idx<probs.size(); ++idx )
        {
            auto prob = 1.0f * probs[ idx ] / totalInfections;
            if( prob )
            {
                entropy += prob * log( prob );
            }
        }
    }
    entropy *= -1.0;
    Accumulate(_genetic_entropy_label, entropy );

    total_infections = 0; // different variable!
    LOG_DEBUG_F( "%d: total_infections = %d\n", __LINE__, total_infections );
}

void
ReportPolio::postProcessAccumulatedData()
{
    LOG_DEBUG( "postProcessAccumulatedData\n" );
    ReportEnvironmental::postProcessAccumulatedData();

    // pass through normalization
    // order matters, since we're changing channels in place (not like old way)
    normalizeChannel(_aoi_label, _tot_prev_label);

    normalizeChannel(_tot_prev_label, _stat_pop_label);
    normalizeChannel(_wpv1_prev_label, _stat_pop_label);
    normalizeChannel(_wpv2_prev_label, _stat_pop_label);
    normalizeChannel(_wpv3_prev_label, _stat_pop_label);
    normalizeChannel(_vdpv1_prev_label, _stat_pop_label);
    normalizeChannel(_vdpv2_prev_label, _stat_pop_label);
    normalizeChannel(_vdpv3_prev_label, _stat_pop_label);
    normalizeChannel(_immunity_label, _new_infections_label);
    normalizeChannel(_seroprevalence_label_1, _stat_pop_label);
    normalizeChannel(_seroprevalence_label_2, _stat_pop_label);
    normalizeChannel(_seroprevalence_label_3, _stat_pop_label);

    channelDataMap.ExponentialValues( _immunity_label );

    // logprev for additional channels would go here
    addDerivedLogScaleSummaryChannel(_tot_prev_label, _log_tot_prev_label);
    addDerivedLogScaleSummaryChannel(_wpv1_prev_label, _wpv1_log_prev_label);
    addDerivedLogScaleSummaryChannel(_wpv2_prev_label, _wpv2_log_prev_label);
    addDerivedLogScaleSummaryChannel(_wpv3_prev_label, _wpv3_log_prev_label);
    addDerivedLogScaleSummaryChannel(_vdpv1_prev_label, _vdpv1_log_prev_label);
    addDerivedLogScaleSummaryChannel(_vdpv2_prev_label, _vdpv2_log_prev_label);
    addDerivedLogScaleSummaryChannel(_vdpv3_prev_label, _vdpv3_log_prev_label);
}

void
ReportPolio::populateSummaryDataUnitsMap(
    std::map<std::string, std::string> &units_map
)
{
    ReportEnvironmental::populateSummaryDataUnitsMap(units_map);
    
    // Additional malaria channels
    units_map[_wpv1_prev_label]                 = _infected_fraction_label;
    units_map[_wpv2_prev_label]                 = _infected_fraction_label;
    units_map[_wpv3_prev_label]                 = _infected_fraction_label;
    units_map[_vdpv1_prev_label]                = _infected_fraction_label;
    units_map[_vdpv2_prev_label]                = _infected_fraction_label;
    units_map[_vdpv3_prev_label]                = _infected_fraction_label;

    units_map[_wpv1_log_prev_label]            = _log_prev_label;
    units_map[_wpv2_log_prev_label]            = _log_prev_label;
    units_map[_wpv3_log_prev_label]            = _log_prev_label;
    units_map[_vdpv1_log_prev_label]           = _log_prev_label;
    units_map[_vdpv2_log_prev_label]           = _log_prev_label;
    units_map[_vdpv3_log_prev_label]           = _log_prev_label;
}

void
ReportPolio::LogIndividualData(
    IndividualHuman * individual
)
{
    ReportEnvironmental::LogIndividualData( individual );
    IIndividualHumanPolio* polio_individual = nullptr;
    if( individual->QueryInterface( GET_IID( IIndividualHumanPolio ), (void**)&polio_individual ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualPolio", "IndividualHuman" );
    }

    auto mc_weight = individual->GetMonteCarloWeight();
    NewInfectionState::_enum tmpNewInfFlag = individual->GetNewInfectionState();
    if(tmpNewInfFlag == NewInfectionState::NewlyDetected || tmpNewInfFlag == NewInfectionState::NewAndDetected)
    {
        int paralysisVirusTypeMask = polio_individual->GetParalysisVirusTypeMask();

        for( int type = 0; type <= N_POLIO_VIRUS_TYPES; type++ )
        {
            if( paralysisVirusTypeMask & (1 << type) )
            {
                cumulative_paralytic_cases[type] += mc_weight;
            }
        }

        total_cumulative_paralytic_cases += mc_weight;
    }

    auto infections = individual->GetInfections();
    for( auto it = infections.cbegin(); it != infections.cend(); ++it )
    {
        auto * infection = *it;
        // TBD: Nasty casting must be replaced by QI
        infectionStrainIdCounts[ ((InfectionPolio*)infection)->GetGeneticID() ] += mc_weight;
        if( tmpNewInfFlag == NewInfectionState::NewInfection )
        {
            float immunity = ((InfectionPolio*)infection)->GetMusocalImmunity(); // (sic!)
            if( immunity )
            {
                immunityAtInfection += log( immunity ) * mc_weight;
            }
        }
    }

    auto suscept_reporter = polio_individual->GetSusceptibilityReporting();
    for( int seroId = 0; seroId < N_POLIO_SEROTYPES; ++seroId )
    {
        seroprevalence[ seroId ] += suscept_reporter->IsSeropositive( seroId ) * mc_weight;
    }

    auto infStrainsArray = suscept_reporter->GetInfectionStrains();

    Accumulate(_wpv1_prev_label,  infStrainsArray[0]*mc_weight);
    Accumulate(_wpv2_prev_label,  infStrainsArray[1]*mc_weight);
    Accumulate(_wpv3_prev_label,  infStrainsArray[2]*mc_weight);
    Accumulate(_vdpv1_prev_label, infStrainsArray[3]*mc_weight);
    Accumulate(_vdpv2_prev_label, infStrainsArray[4]*mc_weight);
    Accumulate(_vdpv3_prev_label, infStrainsArray[5]*mc_weight);

    auto num_infections = std::accumulate( infStrainsArray, infStrainsArray + 6, 0 ) * mc_weight;
    LOG_DEBUG_F( "%d: infections = %d\n", __LINE__, num_infections );
    Accumulate(_tot_prev_label, num_infections );
    total_infections += num_infections;
    if( tmpNewInfFlag == NewInfectionState::NewInfection ||
        tmpNewInfFlag == NewInfectionState::NewAndDetected )
    {
        auto newInfStrainsArray = suscept_reporter->GetNewInfectionsByStrain();

        if(newInfStrainsArray[DiseaseSusceptibleReportPoliovirusType] > 0 && polio_individual->GetSusceptibleStatus(DiseaseSusceptibleReportPoliovirusType) )
        {
            /*if(individual->GetAge() < 5*DAYSPERYEAR)
            {
                newDiseaseSusceptibleInfectionsUnder5 += mc_weight; 
            }
            else
            {
                newDiseaseSusceptibleInfectionsOver5 += mc_weight; 
            }*/

            // Accumulate disease susceptible infections in the reporter
            cumulativeDiseaseSusceptibleInfections += mc_weight; 
        }

    }
}

void
ReportPolio::LogNodeData(
    INodeContext * pNC
)
{
    ReportEnvironmental::LogNodeData( pNC );
    const INodePolio * pPolioNode = nullptr; // TBD: Use limited read-only interface, not full NodePolio
    if( pNC->QueryInterface( GET_IID( INodePolio), (void**) &pPolioNode ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodePolio", "INodeContext" );
    }
    LOG_DEBUG_F( "mean age = %f, total_infections = %d\n", pPolioNode->GetMeanAgeInfection(), total_infections );
    Accumulate( _aoi_label, pPolioNode->GetMeanAgeInfection() * total_infections ); // weight the age of infection by the number of infections in the node. global normalization happens in SimulationPolio

    total_infections = 0;
}

}

#endif // ENABLE_POLIO
