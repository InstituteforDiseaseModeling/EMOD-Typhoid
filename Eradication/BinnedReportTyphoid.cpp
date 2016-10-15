/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TYPHOID

#include "BinnedReportTyphoid.h"

#include <map>
#include <string>

#include "BoostLibWrapper.h"
#include "Environment.h"
#include "Exceptions.h"
#include "Sugar.h"
#include "TyphoidDefs.h"
#include "IndividualTyphoid.h"
#include "Common.h"

using namespace std;
using namespace json;
//using namespace Kernel;

namespace Kernel {

// Module name for logging
static const char * _module = "BinnedReportTyphoid"; 

static const std::string _num_chronic_carriers_label     = "Number of Chronic Carriers";
static const std::string _num_subclinic_infections_label = "Number of New Sub-Clinical Infections";
static const std::string _num_acute_infections_label     = "Number of New Acute Infections";

Kernel::IReport*
BinnedReportTyphoid::CreateReport()
{
    return new BinnedReportTyphoid();
}

// Derived constructor calls base constructor to initialized reduced timesteps etc. 
BinnedReportTyphoid::BinnedReportTyphoid() 
    : BinnedReport()
    , carrier_bins( nullptr )
    , subclinical_bins( nullptr )
    , acute_bins( nullptr )
{
    LOG_DEBUG( "BinnedReportTyphoid ctor\n" );
    _num_age_bins = 100;

    _age_bin_friendly_names.resize( _num_age_bins );
    memset( _age_bin_upper_values, 0, sizeof( float ) * 100 );
    for( int idx = 0; idx < _num_age_bins ; idx++ )
    {
        _age_bin_upper_values[idx] = DAYSPERYEAR*(idx+1);
        _age_bin_friendly_names[idx] = std::to_string( idx );
    }
}

BinnedReportTyphoid::~BinnedReportTyphoid()
{
    delete[] carrier_bins;
    delete[] subclinical_bins;
    delete[] acute_bins;
}

void BinnedReportTyphoid::initChannelBins()
{
    carrier_bins = new float[num_total_bins];
    subclinical_bins = new float[num_total_bins];
    acute_bins = new float[num_total_bins];

    BinnedReport::initChannelBins();

    clearChannelsBins();
}

void BinnedReportTyphoid::clearChannelsBins()
{
    memset(carrier_bins, 0, num_total_bins * sizeof(float));
    memset(subclinical_bins, 0, num_total_bins * sizeof(float));
    memset(acute_bins, 0, num_total_bins * sizeof(float));
}

void BinnedReportTyphoid::EndTimestep( float currentTime, float dt )
{
    if (currentTime<6570){
        num_timesteps--;
        return;
    }

    Accumulate( _num_chronic_carriers_label,     carrier_bins );
    Accumulate( _num_subclinic_infections_label, subclinical_bins );
    Accumulate( _num_acute_infections_label,     acute_bins );

    BinnedReport::EndTimestep( currentTime, dt );

    clearChannelsBins();
}

void  BinnedReportTyphoid::LogIndividualData( Kernel::IndividualHuman * individual )
{
    LOG_DEBUG( "BinnedReportTyphoid::LogIndividualData\n" );

    // Look ma, I can copy-paste code...(from ReportTyphoid.cpp, let's refactor)
    Kernel::IIndividualHumanTyphoid* typhoid_individual = NULL;
    if( individual->QueryInterface( GET_IID( Kernel::IIndividualHumanTyphoid ), (void**)&typhoid_individual ) != Kernel::s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualTyphoid", "IndividualHuman" );
    }

    auto mc_weight = individual->GetMonteCarloWeight();
    int bin_index = calcBinIndex(individual);

    if( typhoid_individual->IsChronicCarrier() )
    {
        carrier_bins[ bin_index ] += mc_weight;
    }

    if( individual->IsInfected() )
    {
        if( typhoid_individual->IsSubClinical() )
        {
            subclinical_bins[ bin_index ] += mc_weight;
        }
        else if( typhoid_individual->IsAcute() )
        {
            acute_bins[ bin_index ] += mc_weight;
        }
    }

    BinnedReport::LogIndividualData(individual);
}

void BinnedReportTyphoid::postProcessAccumulatedData()
{
}

}

#endif // ENABLE_TYPHOID
