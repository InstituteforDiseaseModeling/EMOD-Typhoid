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

#include <numeric> // for std::accumulate
#include "ReportTyphoid.h" // for base class
#include "NodeTyphoid.h" // for base class
#include "Debug.h" // for base class

static const char * _module = "ReportTyphoid";


namespace Kernel {

static const std::string _num_chronic_carriers_label     = "Number of Chronic Carriers";
static const std::string _num_subclinic_infections_label = "Number of New Sub-Clinical Infections";
static const std::string _num_acute_infections_label     = "Number of New Acute Infections";


GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportTyphoid,ReportTyphoid)


ReportTyphoid::ReportTyphoid()
{
}

bool ReportTyphoid::Configure( const Configuration * inputJson )
{
    bool ret = JsonConfigurable::Configure( inputJson );
    return ret ;
}

void ReportTyphoid::EndTimestep( float currentTime, float dt )
{
    ReportEnvironmental::EndTimestep( currentTime, dt );
    
    // Make sure we push at least one zero per timestep
    Accumulate( _num_chronic_carriers_label, 0 );
    Accumulate( _num_subclinic_infections_label, 0 );
    Accumulate( _num_acute_infections_label, 0 );
}

void
ReportTyphoid::postProcessAccumulatedData()
{
    LOG_DEBUG( "postProcessAccumulatedData\n" );
    ReportEnvironmental::postProcessAccumulatedData();

    // pass through normalization
    // order matters, since we're changing channels in place (not like old way)
    //normalizeChannel(_aoi_label, _tot_prev_label);

}

void
ReportTyphoid::populateSummaryDataUnitsMap(
    std::map<std::string, std::string> &units_map
)
{
    ReportEnvironmental::populateSummaryDataUnitsMap(units_map);
    
    // Additional malaria channels
    //units_map[_wpv1_prev_label]                 = _infected_fraction_label;
}

void
ReportTyphoid::LogIndividualData(
    IIndividualHuman * individual
)
{
    ReportEnvironmental::LogIndividualData( individual );
    IIndividualHumanTyphoid* typhoid_individual = NULL;
    if( individual->QueryInterface( GET_IID( IIndividualHumanTyphoid ), (void**)&typhoid_individual ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualTyphoid", "IndividualHuman" );
    }

    auto mc_weight = individual->GetMonteCarloWeight();
    if( typhoid_individual->IsChronicCarrier( false ) )
    {
        Accumulate( _num_chronic_carriers_label, mc_weight );
    }

    if( individual->IsInfected() )
    {
        if( typhoid_individual->IsSubClinical() )
        {
            Accumulate( _num_subclinic_infections_label, mc_weight );
        }
        else if( typhoid_individual->IsAcute() )
        {
            Accumulate( _num_acute_infections_label, mc_weight );
        }
    }
}

void
ReportTyphoid::LogNodeData(
    INodeContext * pNC
)
{
    ReportEnvironmental::LogNodeData( pNC );
    const INodeTyphoid * pTyphoidNode = NULL; // TBD: Use limited read-only interface, not full NodeTyphoid
    if( pNC->QueryInterface( GET_IID( INodeTyphoid), (void**) &pTyphoidNode ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodeTyphoid", "INodeContext" );
    }
#warning "HACK: Commented out code until I merge Typhoid & mainline solutions for GetTotalContagion."
#if 0
    auto contactContagionPop = pNC->GetTotalContagion();
    Accumulate( "Contact Contagion Population", contactContagionPop["contact"] );
    Accumulate( "Environmental Contagion Population", contactContagionPop["environmental"] );
    //Accumulate( _aoi_label, pTyphoidNode->GetMeanAgeInfection() * total_infections ); // weight the age of infection by the number of infections in the node. global normalization happens in SimulationTyphoid
#endif
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(ReportTyphoid)
template<class Archive>
void serialize(Archive &ar, ReportTyphoid& report, const unsigned int v)
{
    boost::serialization::void_cast_register<ReportTyphoid,IReport>();
    ar &boost::serialization::base_object<Report>(report);
}
#endif

}

#endif // ENABLE_TYPHOID