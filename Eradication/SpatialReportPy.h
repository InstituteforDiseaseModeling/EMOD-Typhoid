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

#pragma once

#include <list>
#include <map>
#include <vector>
#include <string>
#include <fstream>

#include "SpatialReport.h"
#include "BoostLibWrapper.h"

namespace Kernel {

class SpatialReportPy : public SpatialReport
{
    GET_SCHEMA_STATIC_WRAPPER(SpatialReportPy)

public:
    static IReport* CreateReport();
    virtual ~SpatialReportPy() { }

    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void LogNodeData( Kernel::INodeContext * pNC );

protected:
    SpatialReportPy();

    virtual void postProcessAccumulatedData();

    virtual void populateChannelInfos(tChanInfoMap &channel_infos);

    // counters for LogIndividualData stuff 

private:
#if USE_BOOST_SERIALIZATION
    friend class ::boost::serialization::access;
    template<class Archive>
    friend void serialize(Archive &ar, SpatialReportPy& report, const unsigned int v);
#endif
};
}
