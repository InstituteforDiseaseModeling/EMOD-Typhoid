/*****************************************************************************

Copyright (c) 2014 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

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
