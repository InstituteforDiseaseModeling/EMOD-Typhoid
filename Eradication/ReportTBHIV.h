/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Types.h"
#include "ReportTB.h" // for base class

namespace Kernel {

class ReportTBHIV : public ReportTB
{
public:
    ReportTBHIV();

    static IReport* ReportTBHIV::CreateReport()
    {
        return new ReportTBHIV();
    }
    virtual void EndTimestep( float currentTime, float dt );
    virtual void LogNodeData( INodeContext * pNC );
    virtual void LogIndividualData( IIndividualHuman* individual );
    virtual void BeginTimestep();

protected:
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map );
    virtual void postProcessAccumulatedData();

private:
    float m_allTB_HIV_persons;
    float m_active_TB_HIV_persons;
    float m_latent_TB_HIV_persons;
    float m_HIV_persons;
    float m_CD4count_TB_and_HIV_pos_persons;
    float m_TB_active_500_350;
    float m_TB_active_below_350;
    float m_TB_active_above_500;
};

}
