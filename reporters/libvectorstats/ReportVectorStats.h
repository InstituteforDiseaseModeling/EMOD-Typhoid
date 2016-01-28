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

#include "BaseTextReportEvents.h"
#include "IVectorMigrationReporting.h"

namespace Kernel
{
    class ReportVectorStats : public BaseTextReportEvents, public IVectorMigrationReporting
    {
    public:
        ReportVectorStats();
        virtual ~ReportVectorStats();

        // BaseEventReport
        virtual bool Configure( const Configuration* );
        //virtual void BeginTimestep() ;

        virtual void UpdateEventRegistration( float currentTime, 
                                              float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList );

        virtual std::string GetHeader() const;
        virtual void LogNodeData( Kernel::INodeContext * pNC );
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, 
                                    const std::string& StateChange );

        // IVectorMigrationReporting
        virtual void LogVectorMigration( ISimulationContext* pSim, 
                                         float currentTime, 
                                         const suids::suid& nodeSuid, 
                                         IVectorCohort* pivc );
    private:
        int migration_count_local ;
        int migration_count_regional ;
        std::vector<std::string> species_list ;
    };
}
