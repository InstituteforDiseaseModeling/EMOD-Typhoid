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

#ifdef ENABLE_POLIO

#include <numeric> // for std::accumulate
#include "ReportPyDemo.h" // for base class
#include "NodePyDemo.h" // for base class
#include "Debug.h" // for base class

static const char * _module = "ReportPyDemo";


namespace Kernel {

static const std::string _num_chronic_carriers_label     = "Number of Chronic Carriers";
static const std::string _num_subclinic_infections_label = "Number of New Sub-Clinical Infections";
static const std::string _num_acute_infections_label     = "Number of New Acute Infections";


GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportPyDemo,ReportPyDemo)


ReportPyDemo::ReportPyDemo()
{
}

bool ReportPyDemo::Configure( const Configuration * inputJson )
{
    bool ret = JsonConfigurable::Configure( inputJson );
    return ret ;
}

void ReportPyDemo::EndTimestep( float currentTime, float dt )
{
    ReportEnvironmental::EndTimestep( currentTime, dt );
    
	// Make sure we push at least one zero per timestep
	Accumulate( _num_chronic_carriers_label, 0 );
	Accumulate( _num_subclinic_infections_label, 0 );
	Accumulate( _num_acute_infections_label, 0 );
}

void
ReportPyDemo::postProcessAccumulatedData()
{
    LOG_DEBUG( "postProcessAccumulatedData\n" );
    ReportEnvironmental::postProcessAccumulatedData();

    // pass through normalization
    // order matters, since we're changing channels in place (not like old way)
    //normalizeChannel(_aoi_label, _tot_prev_label);

}

void
ReportPyDemo::populateSummaryDataUnitsMap(
    std::map<std::string, std::string> &units_map
)
{
    ReportEnvironmental::populateSummaryDataUnitsMap(units_map);
    
    // Additional malaria channels
    //units_map[_wpv1_prev_label]                 = _infected_fraction_label;
}

void
ReportPyDemo::LogIndividualData(
    IndividualHuman * individual
)
{
    ReportEnvironmental::LogIndividualData( individual );
    IIndividualHumanPyDemo* typhoid_individual = NULL;
    if( individual->QueryInterface( GET_IID( IIndividualHumanPyDemo ), (void**)&typhoid_individual ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualPyDemo", "IndividualHuman" );
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
ReportPyDemo::LogNodeData(
    INodeContext * pNC
)
{
    ReportEnvironmental::LogNodeData( pNC );
    const INodePyDemo * pPyDemoNode = NULL; // TBD: Use limited read-only interface, not full NodePyDemo
    if( pNC->QueryInterface( GET_IID( INodePyDemo), (void**) &pPyDemoNode ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodePyDemo", "INodeContext" );
    }

    auto contactContagionPop = pNC->GetTotalContagion();
    Accumulate( "Contact Contagion Population", contactContagionPop["contact"] );
    Accumulate( "Environmental Contagion Population", contactContagionPop["environmental"] );
    //Accumulate( _aoi_label, pPyDemoNode->GetMeanAgeInfection() * total_infections ); // weight the age of infection by the number of infections in the node. global normalization happens in SimulationPyDemo
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(ReportPyDemo)
template<class Archive>
void serialize(Archive &ar, ReportPyDemo& report, const unsigned int v)
{
    boost::serialization::void_cast_register<ReportPyDemo,IReport>();
    ar &boost::serialization::base_object<Report>(report);
}
#endif

}

#endif // ENABLE_POLIO
