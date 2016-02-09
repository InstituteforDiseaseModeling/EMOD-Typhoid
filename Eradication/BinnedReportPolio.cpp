/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "BinnedReportPolio.h"

#include <map>
#include <string>

#include "BoostLibWrapper.h"
#include "Environment.h"
#include "Exceptions.h"
#include "Sugar.h"
#include "PolioDefs.h"
#include "IndividualPolio.h"
#include "Common.h"

using namespace std;
using namespace json;

// Module name for logging
static const char * _module = "BinnedReportPolio"; 

Kernel::IReport*
BinnedReportPolio::CreateReport()
{
    return new BinnedReportPolio();
}

// Derived constructor calls base constructor to initialized reduced timesteps etc. 
BinnedReportPolio::BinnedReportPolio() 
    : BinnedReport()
    , wpv1_bins(nullptr)
    , wpv2_bins(nullptr)
    , wpv3_bins(nullptr)
    , vrpv1_bins(nullptr)
    , vrpv2_bins(nullptr)
    , vrpv3_bins(nullptr)
    , new_paralytic_cases(nullptr)
{
    LOG_DEBUG( "BinnedReportPolio ctor\n" );
}

BinnedReportPolio::~BinnedReportPolio()
{
    delete[] wpv1_bins;
    delete[] wpv2_bins;
    delete[] wpv3_bins;
    delete[] vrpv1_bins;
    delete[] vrpv2_bins;
    delete[] vrpv3_bins;
    delete[] new_paralytic_cases;
}

void BinnedReportPolio::initChannelBins()
{
    BinnedReport::initChannelBins();

    wpv1_bins = new float[num_total_bins];
    wpv2_bins = new float[num_total_bins];
    wpv3_bins = new float[num_total_bins];
    vrpv1_bins = new float[num_total_bins];
    vrpv2_bins = new float[num_total_bins];
    vrpv3_bins = new float[num_total_bins];
    new_paralytic_cases = new float[num_total_bins];

    clearChannelsBins();
}

void BinnedReportPolio::clearChannelsBins()
{
    memset(wpv1_bins, 0, num_total_bins * sizeof(float));
    memset(wpv2_bins, 0, num_total_bins * sizeof(float));
    memset(wpv3_bins, 0, num_total_bins * sizeof(float));
    memset(vrpv1_bins, 0, num_total_bins * sizeof(float));
    memset(vrpv2_bins, 0, num_total_bins * sizeof(float));
    memset(vrpv3_bins, 0, num_total_bins * sizeof(float));
    memset(new_paralytic_cases, 0, num_total_bins * sizeof(float));
}

void BinnedReportPolio::EndTimestep( float currentTime, float dt )
{
    Accumulate("WPV1 Positive", wpv1_bins);
    Accumulate("WPV2 Positive", wpv2_bins);
    Accumulate("WPV3 Positive", wpv3_bins);
    Accumulate("VDPV1 Positive", vrpv1_bins);
    Accumulate("VDPV2 Positive", vrpv2_bins);
    Accumulate("VDPV3 Positive", vrpv3_bins);
    Accumulate("New Paralytic Cases", new_paralytic_cases);

    BinnedReport::EndTimestep( currentTime, dt );

    clearChannelsBins();
}

void  BinnedReportPolio::LogIndividualData( Kernel::IIndividualHuman* individual )
{
    LOG_DEBUG( "BinnedReportPolio::LogIndividualData\n" );

    BinnedReport::LogIndividualData(individual);

    // Get individual weight and bin variables
    float mc_weight    = (float)individual->GetMonteCarloWeight();

    int bin_index = calcBinIndex(individual);

    if (individual->IsInfected())
    {
        NewInfectionState::_enum nis = individual->GetNewInfectionState();

        if(nis == NewInfectionState::NewAndDetected || nis == NewInfectionState::NewlyDetected)
            new_paralytic_cases[bin_index] += mc_weight;

        const Kernel::IIndividualHumanPolio* individual_polio = nullptr;
        if( individual->QueryInterface( GET_IID( Kernel::IIndividualHumanPolio), (void**) &individual_polio ) != Kernel::s_OK )
        {
            throw Kernel::QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualHumanPolio", "IndividualHuman" );
        }

        auto strain_counts = individual_polio->GetSusceptibilityReporting()->GetInfectionStrains();

        wpv1_bins[bin_index] += mc_weight * strain_counts[0];
        wpv2_bins[bin_index] += mc_weight * strain_counts[1];
        wpv3_bins[bin_index] += mc_weight * strain_counts[2];
        vrpv1_bins[bin_index] += mc_weight * strain_counts[3];
        vrpv2_bins[bin_index] += mc_weight * strain_counts[4];
        vrpv3_bins[bin_index] += mc_weight * strain_counts[5];
    }
}

void BinnedReportPolio::postProcessAccumulatedData()
{
    addDerivedCumulativeSummaryChannel("New Paralytic Cases", "Cumulative Paralytic Cases");
    //channelDataMap.erase("New Paralytic Cases");
}

#endif // ENABLE_POLIO
