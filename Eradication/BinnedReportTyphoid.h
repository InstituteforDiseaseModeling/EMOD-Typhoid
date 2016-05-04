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

#include "BinnedReport.h"

namespace Kernel {

class BinnedReportTyphoid : public BinnedReport
{
public:
    static IReport* CreateReport();
    virtual ~BinnedReportTyphoid();

    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void EndTimestep( float currentTime, float dt );

    virtual void postProcessAccumulatedData();

protected:
    BinnedReportTyphoid();

    virtual void initChannelBins();
    void clearChannelsBins();

    float *carrier_bins;
    float *subclinical_bins;
    float *acute_bins;
    // channels specific to this particular report-type
};

}