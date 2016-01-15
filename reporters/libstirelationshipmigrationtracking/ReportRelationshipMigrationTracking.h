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

namespace Kernel
{
    class ReportRelationshipMigrationTracking : public BaseTextReportEvents
    {
    public:
        ReportRelationshipMigrationTracking();
        virtual ~ReportRelationshipMigrationTracking();

        // BaseEventReport
        virtual bool Configure( const Configuration* );

        virtual void UpdateEventRegistration( float currentTime, 
                                              float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList );

        virtual std::string GetHeader() const;
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, 
                                    const std::string& StateChange );
        virtual void LogIndividualData( IIndividualHuman* individual ) ;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const ;
        virtual void EndTimestep( float currentTime, float dt );
        virtual void Reduce();
    private:
        struct MigrationData
        {
            MigrationData() : age_years(-1.0), gender(-1), node_id(-1), migration_type_str() {}

            float age_years ;
            int gender ;
            uint32_t node_id ;
            std::string migration_type_str ;
        };
        float m_EndTime ;
        std::map<long,MigrationData> m_MigrationDataMap ;
    };
}
