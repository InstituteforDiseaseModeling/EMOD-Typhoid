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

#include <sstream>
#include "SimulationEnums.h"
#include "BaseTextReportEvents.h"

#define MAX_AGE (100)

namespace Kernel {
    struct ISimulation;

    class ReportTyphoidByAgeAndGender : public BaseTextReportEvents
    {
        GET_SCHEMA_STATIC_WRAPPER(ReportTyphoidByAgeAndGender)
    public:
        ReportTyphoidByAgeAndGender( const ISimulation *sim = nullptr, float period = 180.0 );
        static IReport* ReportTyphoidByAgeAndGender::Create(const ISimulation * parent, float period) { return new ReportTyphoidByAgeAndGender( parent, period ); }

        // -----------------------------
        // --- BaseTextReportEvents
        // -----------------------------
        virtual bool Configure( const Configuration* inputJson );
        virtual void UpdateEventRegistration( float currentTime, 
                                              float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList );
        virtual std::string GetHeader() const ;
        virtual bool notifyOnEvent(IIndividualHumanEventContext *context, const std::string& StateChange);

        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const ;
        virtual void LogIndividualData( IIndividualHuman* individual );
        virtual void LogNodeData( INodeContext* pNC );

    protected:

    private:

        //const float report_half_period;
        float next_report_time;
        bool doReport;

        float population[Gender::Enum::COUNT][MAX_AGE];         //Gender, Age --> Population
        float infected[Gender::Enum::COUNT][MAX_AGE];           //Gender, Age --> Infected
        float newly_infected[Gender::Enum::COUNT][MAX_AGE];     //Gender, Age --> Newly Infected 

        float chronic[Gender::Enum::COUNT][MAX_AGE];           
        float subClinical[Gender::Enum::COUNT][MAX_AGE];           
        float acute[Gender::Enum::COUNT][MAX_AGE];           
        float prePatent[Gender::Enum::COUNT][MAX_AGE];           

        float chronic_inc[Gender::Enum::COUNT][MAX_AGE];           
        float subClinical_inc[Gender::Enum::COUNT][MAX_AGE];           
        float acute_inc[Gender::Enum::COUNT][MAX_AGE];           
        float prePatent_inc[Gender::Enum::COUNT][MAX_AGE];           

        const ISimulation * _parent;
        float startYear ;                                       // Year to start collecting data
        float stopYear ;                                        // Year to stop  collecting data
        bool is_collecting_data ;
    };

}

