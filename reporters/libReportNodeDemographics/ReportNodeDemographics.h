/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/


#pragma once

#include "BaseTextReport.h"

namespace Kernel
{
    struct NodeData
    {
        NodeData()
        : num_people(0)
        , num_infected(0)
        {
        };

        int num_people;
        int num_infected;
    };

    class ReportNodeDemographics: public BaseTextReport
    {
    public:
        ReportNodeDemographics();
        virtual ~ReportNodeDemographics();

        // BaseEventReport
        virtual bool Configure( const Configuration* );

        virtual std::string GetHeader() const;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const ;
        virtual void LogIndividualData( IIndividualHuman* individual ) ;
        virtual void LogNodeData( INodeContext* pNC );
    private:
        std::vector<float> m_AgeYears;
        std::vector<std::vector<NodeData>> m_Data;
    };
}
