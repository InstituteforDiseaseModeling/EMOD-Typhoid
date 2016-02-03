/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <map>

#include "BaseTextReport.h"
#include "ReportMalaria.h"

namespace Kernel
{
    class ReportMalariaFiltered : public ReportMalaria
    {
    public:
        ReportMalariaFiltered();
        virtual ~ReportMalariaFiltered();

        // ReportMalaria
        virtual bool Configure( const Configuration* );

        virtual void UpdateEventRegistration( float currentTime, 
                                              float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList );
        virtual void BeginTimestep();
        virtual void LogIndividualData( IIndividualHuman* individual ) ;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const ;
        virtual void LogNodeData( INodeContext* pNC );
    private:
        bool IsValidNode( uint32_t externalNodeID ) const;

        std::map<uint32_t,bool> m_NodesToInclude;
        float m_StartDay;
        float m_EndDay;
        bool m_IsValidDay;
    };
}
