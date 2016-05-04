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

#include <functional>
#include <numeric>
#include <map>
#include "BoostLibWrapper.h"

#include "SpatialReportTyphoid.h"
#include "NodeTyphoid.h"
#include "Sugar.h"
#include "Environment.h"
#include "Exceptions.h"
#include "Individual.h"
#include "SimulationConfig.h"
#include "ProgVersion.h"

using namespace std;

static const char * _module = "SpatialReportTyphoid";

namespace Kernel {

GET_SCHEMA_STATIC_WRAPPER_IMPL(SpatialReportTyphoid,SpatialReportTyphoid)

/////////////////////////
// Initialization methods
/////////////////////////
IReport*
SpatialReportTyphoid::CreateReport()
{
    return new SpatialReportTyphoid();
}

SpatialReportTyphoid::SpatialReportTyphoid()
: SpatialReportEnvironmental()
{
}

void SpatialReportTyphoid::populateChannelInfos(tChanInfoMap &channel_infos)
{
    SpatialReportEnvironmental::populateChannelInfos(channel_infos);
}


void SpatialReportTyphoid::LogIndividualData( IIndividualHuman * individual )
{
    SpatialReportEnvironmental::LogIndividualData(individual);

    float monte_carlo_weight = (float)individual->GetMonteCarloWeight();

    NewInfectionState::_enum nis = individual->GetNewInfectionState();
}

void
SpatialReportTyphoid::LogNodeData(
    INodeContext * pNC
)
{
    SpatialReport::LogNodeData(pNC);

    int nodeid = pNC->GetExternalID();

    const INodeTyphoid * pTyphoidNode = NULL;
    if( pNC->QueryInterface( GET_IID( INodeTyphoid), (void**) &pTyphoidNode ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodeTyphoid", "INodeContext" );
    }
}

void
SpatialReportTyphoid::postProcessAccumulatedData()
{
    SpatialReport::postProcessAccumulatedData();

    // pass through normalization
    // order matters, since we're changing channels in place (not like old way)
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(SpatialReport)
template<class Archive>
void serialize(Archive &ar, SpatialReportTyphoid& report, const unsigned int v)
{
    boost::serialization::void_cast_register<SpatialReportTyphoid,IReport>();
    ar & report.timesteps_reduced;
    ar & report.channelDataMap;
    ar & report._nrmSize;
}
#endif

}

#endif // ENABLE_TYPHOID
