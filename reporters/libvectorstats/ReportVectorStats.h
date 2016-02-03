/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

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
