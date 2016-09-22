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
static const std::string _num_enviro_infections_label    = "New Infections By Route (ENVIRONMENT)";
static const std::string _num_contact_infections_label   = "New Infections By Route (CONTACT)";
    

GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportTyphoid,ReportTyphoid)


ReportTyphoid::ReportTyphoid()
: recording( false )
, parent(nullptr)
{
}

bool ReportTyphoid::Configure( const Configuration * inputJson )
{
    initConfigTypeMap( "Inset_Chart_Reporting_Start_Year", &startYear, "Start Year for reporting.", MIN_YEAR, MAX_YEAR, 0.0f );
    initConfigTypeMap( "Inset_Chart_Reporting_Stop_Year", &stopYear, "Stop Year for reporting.", MIN_YEAR, MAX_YEAR, 0.0f );
    bool ret = JsonConfigurable::Configure( inputJson );
    LOG_DEBUG_F( "Read in Start_Year (%f) and Stop_Year(%f).\n", startYear, stopYear );
    if( startYear >= stopYear )
    {
        throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Inset_Chart_Reporting_Start_Year", startYear, "Inset_Chart_Reporting_Stop_Year", stopYear );
    }
    return ret ;
}

void ReportTyphoid::BeginTimestep()
{
    if( recording )
    {
        return ReportEnvironmental::BeginTimestep();
    }
}

void ReportTyphoid::EndTimestep( float currentTime, float dt )
{
    if( recording )
    {

        ReportEnvironmental::EndTimestep( currentTime, dt );

        // Make sure we push at least one zero per timestep
        Accumulate( _num_chronic_carriers_label, 0 );
        Accumulate( _num_subclinic_infections_label, 0 );
        Accumulate( _num_acute_infections_label, 0 );
        Accumulate( _num_enviro_infections_label, 0 );
        Accumulate( _num_contact_infections_label, 0 );
    }

    release_assert( parent );
    
    float currentYear = parent->GetSimulationTime().Year();
    LOG_DEBUG_F( "currentYear = %f.\n", currentYear );
    if( currentYear >= startYear && currentYear < stopYear )
    {
        recording = true;
    }
    else
    {
        recording = false;
    }
    LOG_DEBUG_F( "recording = %d\n", recording );
}

void
ReportTyphoid::postProcessAccumulatedData()
{
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
    if( recording == false ) 
    {
        return;
    }

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

        // Get infection incidence by route
        NewInfectionState::_enum nis = individual->GetNewInfectionState(); 
        LOG_DEBUG_F( "nis = %d\n", ( nis ) );
        if( nis == NewInfectionState::NewAndDetected ||
            nis == NewInfectionState::NewInfection /*||
            nis == NewInfectionState::NewlyDetected*/ )
        {
            auto inf = individual->GetInfections().back();
            StrainIdentity si;
            inf->GetInfectiousStrainID( &si );
            if( si.GetGeneticID() == 0 )
            {
                Accumulate( _num_enviro_infections_label, mc_weight );
            }
            else if( si.GetGeneticID() == 1 )
            {
                Accumulate( _num_contact_infections_label, mc_weight );
            }
        }
    }
}

void
ReportTyphoid::LogNodeData(
    INodeContext * pNC
)
{
    if( parent == nullptr )
    {
        parent = pNC->GetParent();
        LOG_DEBUG_F( "Set parent to %x\n", parent );
    }

    if( recording == false ) 
    {
        return;
    }

    ReportEnvironmental::LogNodeData( pNC );
    const INodeTyphoid * pTyphoidNode = NULL; // TBD: Use limited read-only interface, not full NodeTyphoid
    if( pNC->QueryInterface( GET_IID( INodeTyphoid), (void**) &pTyphoidNode ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodeTyphoid", "INodeContext" );
    }

	auto contagionPop = pNC->GetTotalContagion();
    NonNegativeFloat contactContagionPop = contagionPop["contact"];
    NonNegativeFloat enviroContagionPop = contagionPop["environmental"];
    Accumulate( "Contact Contagion Population", contactContagionPop  );
	Accumulate( "Environmental Contagion Population", enviroContagionPop );
    //Accumulate( _aoi_label, pTyphoidNode->GetMeanAgeInfection() * total_infections ); // weight the age of infection by the number of infections in the node. global normalization happens in SimulationTyphoid
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