/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Report.h"

class ReportEnvironmental : public Report
{
public:
    ReportEnvironmental();
    virtual ~ReportEnvironmental(){};

protected:
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map );

    virtual void AddSEIRWUnits( std::map<std::string, std::string> &units_map ) {}
    virtual void UpdateSEIRW( const Kernel::IndividualHuman * individual, float monte_carlo_weight ) {}
    virtual void AccumulateSEIRW() {}
    virtual void NormalizeSEIRWChannels() {}
};
