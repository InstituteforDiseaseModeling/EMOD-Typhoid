/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <functional>
#include <map>
#include "BoostLibWrapper.h"
// not in boost wrapper???
#include <boost/math/special_functions/fpclassify.hpp>

#include "SpatialReportVector.h"
#include "VectorContexts.h"
#include "VectorPopulation.h"
#include "Sugar.h"
#include "Environment.h"
#include "Exceptions.h"
#include "IIndividualHuman.h"
#include "SimulationConfig.h"
#include "ProgVersion.h"

using namespace std;

static const char * _module = "SpatialReportVector";

namespace Kernel {

GET_SCHEMA_STATIC_WRAPPER_IMPL(SpatialReportVector,SpatialReportVector)

/////////////////////////
// Initialization methods
/////////////////////////
IReport*
SpatialReportVector::CreateReport()
{
    return new SpatialReportVector();
}

SpatialReportVector::SpatialReportVector()
: SpatialReport()
, adult_vectors_info(       "Adult_Vectors",                "")
, infectious_vectors_info(  "Infectious_Vectors",           "infectious fraction")
, daily_eir_info(           "Daily_EIR",                    "infectious bites/day")
, daily_hbr_info(           "Daily_Bites_Per_Human",        "bites/day")
{
}

void SpatialReportVector::Initialize( unsigned int nrmSize )
{
    SpatialReport::Initialize( nrmSize );

    if( infectious_vectors_info.enabled && !adult_vectors_info.enabled )
    {
        LOG_WARN("Infectious_Vectors requires that Adult_Vectors be enabled.  Enabling Adult_Vectors.");
        adult_vectors_info.enabled = true ;
        channelDataMap.IncreaseChannelLength( adult_vectors_info.name, _nrmSize );
    }
}

void SpatialReportVector::populateChannelInfos(tChanInfoMap &channel_infos)
{
    SpatialReport::populateChannelInfos(channel_infos);

    channel_infos[ adult_vectors_info.name ] = &adult_vectors_info;
    channel_infos[ infectious_vectors_info.name ] = &infectious_vectors_info;
    channel_infos[ daily_eir_info.name ] = &daily_eir_info;
    channel_infos[ daily_hbr_info.name ] = &daily_hbr_info;
}

void
SpatialReportVector::LogNodeData(
    Kernel::INodeContext * pNC
)
{
    SpatialReport::LogNodeData(pNC);

    float adult_vectors      = 0;
    float infectious_vectors = 0;
    float daily_eir          = 0;
    float daily_hbr          = 0;

    // We want to get the vector populations from our Node pointer. What a perfect use case for QI-ing...
    INodeVector* pNV = nullptr;
    if( pNC->QueryInterface( GET_IID( INodeVector ), (void**) & pNV ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodeVector", "INodeContext" );
    }

    std::list<VectorPopulation*> vectorPopulations = pNV->GetVectorPopulations();

    // TBD
    for (const auto vectorpopulation : vectorPopulations)
    {
        adult_vectors      += (float)( vectorpopulation->getAdultCount() + vectorpopulation->getInfectedCount() + vectorpopulation->getInfectiousCount() );
        infectious_vectors += (float)( vectorpopulation->getInfectiousCount() );

        daily_eir          += vectorpopulation->GetEIRByPool(Kernel::VectorPoolIdEnum::BOTH_VECTOR_POOLS);
        daily_hbr          += vectorpopulation->GetHBRByPool(Kernel::VectorPoolIdEnum::BOTH_VECTOR_POOLS);
    }

    auto nodeid = pNC->GetExternalID();

    if(adult_vectors_info.enabled)
        Accumulate(adult_vectors_info.name, nodeid, adult_vectors);

    if(infectious_vectors_info.enabled)
        Accumulate(infectious_vectors_info.name, nodeid, infectious_vectors);

    if(daily_eir_info.enabled)
        Accumulate(daily_eir_info.name, nodeid, daily_eir);

    if(daily_hbr_info.enabled)
        Accumulate(daily_hbr_info.name, nodeid, daily_hbr);
}

void SpatialReportVector::postProcessAccumulatedData()
{
    SpatialReport::postProcessAccumulatedData();

    if( infectious_vectors_info.enabled && adult_vectors_info.enabled )
    {
        normalizeChannel(infectious_vectors_info.name, adult_vectors_info.name);
    }
}

#if 0
template<class Archive>
void serialize(Archive &ar, SpatialReport& report, const unsigned int v)
{
    ar & report.timesteps_reduced;
    ar & report.channelDataMap;
    ar & report._nrmSize;
}
#endif

}
