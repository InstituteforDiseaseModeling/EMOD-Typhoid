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

#ifdef ENABLE_PYTHON

#include <functional>
#include <numeric>
#include <map>
#include "BoostLibWrapper.h"

#include "SpatialReportPyDemo.h"
#include "NodePyDemo.h"
#include "Sugar.h"
#include "Environment.h"
#include "Exceptions.h"
#include "Individual.h"
#include "SimulationConfig.h"
#include "ProgVersion.h"

using namespace std;

static const char * _module = "SpatialReportPyDemo";

namespace Kernel {

GET_SCHEMA_STATIC_WRAPPER_IMPL(SpatialReportPyDemo,SpatialReportPyDemo)

/////////////////////////
// Initialization methods
/////////////////////////
IReport*
SpatialReportPyDemo::CreateReport()
{
    return new SpatialReportPyDemo();
}

SpatialReportPyDemo::SpatialReportPyDemo()
: SpatialReportEnvironmental()
{
}

void SpatialReportPyDemo::populateChannelInfos(tChanInfoMap &channel_infos)
{
    SpatialReportEnvironmental::populateChannelInfos(channel_infos);
}


void SpatialReportPyDemo::LogIndividualData( Kernel::IndividualHuman * individual )
{
    SpatialReportEnvironmental::LogIndividualData(individual);

    float monte_carlo_weight = (float)individual->GetMonteCarloWeight();

    NewInfectionState::_enum nis = individual->GetNewInfectionState();
}

void
SpatialReportPyDemo::LogNodeData(
    Kernel::INodeContext * pNC
)
{
    SpatialReport::LogNodeData(pNC);

    int nodeid = pNC->GetExternalID();

    const Kernel::INodePyDemo * pPyDemoNode = NULL;
    if( pNC->QueryInterface( GET_IID( Kernel::INodePyDemo), (void**) &pPyDemoNode ) != Kernel::s_OK )
    {
        throw Kernel::QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pNC", "INodePyDemo", "INodeContext" );
    }
}

void
SpatialReportPyDemo::postProcessAccumulatedData()
{
    SpatialReport::postProcessAccumulatedData();

    // pass through normalization
    // order matters, since we're changing channels in place (not like old way)
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(SpatialReport)
template<class Archive>
void serialize(Archive &ar, SpatialReportPyDemo& report, const unsigned int v)
{
    boost::serialization::void_cast_register<SpatialReportPyDemo,IReport>();
    ar & report.timesteps_reduced;
    ar & report.channelDataMap;
    ar & report._nrmSize;
}
#endif

}

#endif // ENABLE_PYTHON
