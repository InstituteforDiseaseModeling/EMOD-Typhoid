/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BinnedReport.h"

namespace Kernel {

class BinnedReportPolio : public BinnedReport
{
public:
    static IReport* CreateReport();
    virtual ~BinnedReportPolio();

    virtual void LogIndividualData( Kernel::IIndividualHuman* individual );
    virtual void EndTimestep( float currentTime, float dt );

    virtual void postProcessAccumulatedData();

protected:
    BinnedReportPolio();

    virtual void initChannelBins();
    void clearChannelsBins();

    // channels specific to this particular report-type
    float *wpv1_bins;
    float *wpv2_bins;
    float *wpv3_bins;
    float *vrpv1_bins;
    float *vrpv2_bins;
    float *vrpv3_bins;
    float *new_paralytic_cases;
};

}
