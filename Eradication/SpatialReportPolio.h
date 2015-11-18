/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <list>
#include <map>
#include <vector>
#include <string>
#include <fstream>

#include "SpatialReport.h"
#include "BoostLibWrapper.h"

namespace Kernel {

class SpatialReportPolio : public SpatialReport
{
    GET_SCHEMA_STATIC_WRAPPER(SpatialReportPolio)

public:
    static IReport* CreateReport();
    virtual ~SpatialReportPolio() { }

    virtual void Initialize( unsigned int nrmSize );

    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void LogNodeData( Kernel::INodeContext * pNC );

protected:
    SpatialReportPolio();

    virtual void postProcessAccumulatedData();

    virtual void populateChannelInfos(tChanInfoMap &channel_infos);

    ChannelInfo age_infection_info;
    ChannelInfo total_prevalence_info;
    ChannelInfo new_paralytic_cases_info;
    ChannelInfo wpv1_prevalence_info;
    ChannelInfo wpv2_prevalence_info;
    ChannelInfo wpv3_prevalence_info;
    ChannelInfo vrpv1_prevalence_info;
    ChannelInfo vrpv2_prevalence_info;
    ChannelInfo vrpv3_prevalence_info;

    // counters for LogIndividualData stuff
    float new_paralytic_cases;
    unsigned int node_infection_total;
};
}
