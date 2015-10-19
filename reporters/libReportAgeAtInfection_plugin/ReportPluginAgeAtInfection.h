/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BaseChannelReport.h"

class ReportPluginAgeAtInfection : public BaseChannelReport
{
public:

    ReportPluginAgeAtInfection();
    virtual ~ReportPluginAgeAtInfection() { }

    virtual void EndTimestep( );
	virtual void LogNodeData( Kernel::INodeContext * pNC );
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const { return true ; } ;
    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void Reduce();
    virtual void Finalize();

protected:
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map );
    virtual void postProcessAccumulatedData();
    float timestep;
    std::vector<float> ages;
    float sampling_ratio;

    std::map< unsigned int, std::vector<float> > time_age_map;

private:
};

