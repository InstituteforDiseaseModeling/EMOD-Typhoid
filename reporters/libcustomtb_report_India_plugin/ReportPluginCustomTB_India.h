/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Report.h"


class CustomTBReportIndiaMATLAB : public Report
{

public:
    CustomTBReportIndiaMATLAB();
    virtual ~CustomTBReportIndiaMATLAB();

    virtual void Initialize( unsigned int nrmSize );

    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void EndTimestep( float currentTime, float dt );
    virtual void Finalize();

protected:
    // additional channels specific to this report-type
    float new_active_smear_pos;
    //float new_active_smear_neg;
    //float new_active_extrapulm;
    float disease_deaths_smear_pos;
    float new_infections_bins;

};

