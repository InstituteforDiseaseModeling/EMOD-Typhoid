/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BaseChannelReport.h"

class ReportPluginAgeAtInfectionHistogram : public BaseChannelReport
{
public:
    ReportPluginAgeAtInfectionHistogram();
    virtual ~ReportPluginAgeAtInfectionHistogram() { }
    virtual void BeginTimestep();
    virtual void EndTimestep( float currentTime, float dt );
    virtual void LogNodeData( Kernel::INodeContext * pNC );
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const { return true ; } ;
    virtual void LogIndividualData( Kernel::IIndividualHuman* individual );
    virtual void Reduce();
    virtual void Finalize();

protected:
    virtual bool Configure( const Configuration* config);
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map );
    virtual void postProcessAccumulatedData();
    int  GetAgeBin(double age);

    float time_since_last_report;
    float reporting_interval_in_years;

    std::vector< float > temp_binned_accumulated_counts;
    std::vector< std::vector< float > > binned_accumulated_counts;

    std::vector< float > age_bin_upper_edges_in_years;

private:
};

