/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BinnedReport.h"

class CustomTBReportMATLAB : public BinnedReport
{
public:
    CustomTBReportMATLAB();
    virtual ~CustomTBReportMATLAB();

    virtual void Initialize( unsigned int nrmSize );

    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void EndTimestep( float currentTime, float dt );
    virtual void Finalize();

protected:
    virtual void initChannelBins();
    void CustomTBReportMATLAB::clearChannelsBins();

    // additional channels specific to this report-type
    float *disease_deaths_bins;
    float *new_infections_bins;
};