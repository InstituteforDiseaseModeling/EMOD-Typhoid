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

#include "stdafx.h"

#include <iomanip> // setprecision
#include <iostream> // defaultfloat
#include "Debug.h"
#include "FileSystem.h"
#include "ReportTyphoidByAgeAndGender.h"
#include "NodeTyphoid.h"
#include "SimulationTyphoid.h" // for base_year
#include "SusceptibilityTyphoid.h"
#include "InfectionTyphoid.h"
#include "TyphoidInterventionsContainer.h"
#include "NodeLevelHealthTriggeredIV.h"
#include "ISimulation.h"

static const char* _module = "ReportTyphoidByAgeAndGender";

namespace Kernel 
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(ReportTyphoidByAgeAndGender,ReportTyphoidByAgeAndGender)

    ReportTyphoidByAgeAndGender::ReportTyphoidByAgeAndGender( const ISimulation * parent, float Period )
        : BaseTextReportEvents("ReportTyphoidByAgeAndGender.csv")
        , next_report_time(0)
        , doReport( false )
        , _parent( parent )
        , startYear(0.0)
        , stopYear(FLT_MAX)
        , is_collecting_data(false)
    {
        //eventTriggerList.push_back( IndividualEventTriggerType::DiseaseDeaths );
        //eventTriggerList.push_back( IndividualEventTriggerType::NonDiseaseDeaths );
        eventTriggerList.push_back( "NewInfectionEvent" );

        ZERO_ARRAY( population );
        ZERO_ARRAY( infected );
        ZERO_ARRAY( newly_infected );
        ZERO_ARRAY( chronic );
        ZERO_ARRAY( subClinical );
        ZERO_ARRAY( acute );
        ZERO_ARRAY( prePatent );
        ZERO_ARRAY( chronic_inc );
        ZERO_ARRAY( subClinical_inc );
        ZERO_ARRAY( acute_inc );
        ZERO_ARRAY( prePatent_inc );
    }

    bool ReportTyphoidByAgeAndGender::Configure( const Configuration* inputJson )
    {
#define Report_Typhoid_ByAgeAndGender_Start_Year_DESC_TEXT "TBD"
#define Report_Typhoid_ByAgeAndGender_Stop_Year_DESC_TEXT "TBD"
        initConfigTypeMap( "Report_Typhoid_ByAgeAndGender_Start_Year", &startYear, Report_Typhoid_ByAgeAndGender_Start_Year_DESC_TEXT, 0.0f, FLT_MAX, 0.0f );
        initConfigTypeMap( "Report_Typhoid_ByAgeAndGender_Stop_Year",  &stopYear,  Report_Typhoid_ByAgeAndGender_Stop_Year_DESC_TEXT,  0.0f, FLT_MAX, FLT_MAX );

        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret )
        {
            if( startYear < SimulationTyphoid::base_year )
            {
                startYear = SimulationTyphoid::base_year ;
            }
            if( startYear >= stopYear )
            {
                 throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                                                         "Report_Typhoid_ByAgeAndGender_Start_Year", startYear, 
                                                         "Report_Typhoid_ByAgeAndGender_Stop_Year", stopYear );
            }
        }

        return ret ;
    }

    void ReportTyphoidByAgeAndGender::UpdateEventRegistration( float currentTime, 
                                                           float dt, 
                                                           std::vector<INodeEventContext*>& rNodeEventContextList )
    {
        // not enforcing simulation to be not null in constructor so one can create schema with it null
        release_assert( _parent );

        float current_year = _parent->GetSimulationTime().Year() ;
        if( !is_collecting_data && (startYear <= current_year) && (current_year < stopYear) )
        {
            BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList );
            is_collecting_data = true ;

            release_assert( SimulationTyphoid::base_year > 0 );
            next_report_time = DAYSPERYEAR*(startYear - SimulationTyphoid::base_year) + DAYSPERYEAR - dt; // / 2.0f ;
            // e.g., Suppose we started sim in 1940, and want to report from 1943 through 1944. dt=1
            //       nrt = 365 * ( 1943.0 - 1940.0 ) +365 - 1
            //           = 365 * 4 - 0.5
            //           = 1459.0 aka "Dec 31 of this year"
            LOG_INFO_F( "Starting to collect data now, next_report_time = %f\n", (float) next_report_time );
        }
        else if( is_collecting_data && (_parent->GetSimulationTime().Year() > stopYear) )
        {
            UnregisterAllNodes();
            is_collecting_data = false ;
        }

        if( is_collecting_data )
        {
            // Figure out when to set doReport to true.  doReport is true for those
            // timesteps where we take a snapshot, i.e., as a function of the
            // half-year offset and full-year periodicity or whatever. In simplest case,
            // it writes out the report (of data that's been collected) 1x per year.
            doReport = false;
            LOG_DEBUG_F( "%s: Setting doReport to false\n", __FUNCTION__ );

            if( currentTime >= next_report_time ) 
            {
                next_report_time += DAYSPERYEAR;

                LOG_DEBUG_F( "Setting doReport to true .\n" );
                doReport = true;
            }
        }
    }

    bool ReportTyphoidByAgeAndGender::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return is_collecting_data; //  && doReport; 
    }

    std::string ReportTyphoidByAgeAndGender::GetHeader() const
    {
        std::stringstream header ;
        header << "Time Of Report (Year)"   << ", "
               << "NodeId"           << ", "
               << "Gender"           << ", "
               << "Age"              << ", "
               << "Population"       << ", "
               << "Infected"         << ", "
               << "Newly Infected"   << ", "
               << "Chronic (Prev)"   << ", "
               << "Sub-Clinical (Prev)" << ", "
               << "Acute (Prev)"     << ", "
               << "Pre-Patent (Prev)"     << ", "
               << "Chronic (Inc) "   << ", "
               << "Sub-Clinical (Inc)" << ", "
               << "Acute (Inc)"      << ", "
               << "Pre-Patent (Inc)"      << ", "
               ;

        return header.str();
    }

    void ReportTyphoidByAgeAndGender::LogNodeData( INodeContext* pNC )
    {
        if( (is_collecting_data == false) || (doReport == false) )
        {
            return;
        }
        //LOG_DEBUG_F( "%s: doReport = %d\n", __FUNCTION__, doReport );
        LOG_INFO_F( "Writing accumulated data to disk for this reporting period.\n" );

        float year = _parent->GetSimulationTime().Year();

        int nodeId = pNC->GetExternalID();

        for( int gender = 0; gender < Gender::Enum::COUNT; gender++ ) 
        {
            for( int age_bin = 0; age_bin < MAX_AGE; age_bin++ ) 
            {
                GetOutputStream() 
#if defined(WIN32)
                                  << std::fixed << std::setprecision(3) 
#endif
                                  << year
#if defined(WIN32)
                                  << std::defaultfloat
#endif
                                  << ","<< nodeId 
                                  << "," << gender    // Kernel::Gender::pairs::lookup_key(gender)
                                  << "," << age_bin
                                  << "," << population[gender][age_bin]
                                  << "," << infected[gender][age_bin]
                                  << "," << newly_infected[gender][age_bin]
                                  << "," << chronic[gender][age_bin]
                                  << "," << subClinical[gender][age_bin]
                                  << "," << acute[gender][age_bin]
                                  << "," << prePatent[gender][age_bin]
                                  << "," << chronic_inc[gender][age_bin]
                                  << "," << subClinical_inc[gender][age_bin]
                                  << "," << acute_inc[gender][age_bin]
                                  << "," << prePatent_inc[gender][age_bin]
                                  << endl;
            }
        }

        ZERO_ARRAY( population );
        ZERO_ARRAY( infected );
        ZERO_ARRAY( newly_infected );
        ZERO_ARRAY( chronic );
        ZERO_ARRAY( subClinical );
        ZERO_ARRAY( acute );
        ZERO_ARRAY( prePatent );
        ZERO_ARRAY( chronic_inc );
        ZERO_ARRAY( subClinical_inc );
        ZERO_ARRAY( acute_inc );
        ZERO_ARRAY( prePatent_inc );
    }

    void ReportTyphoidByAgeAndGender::LogIndividualData( IIndividualHuman* individual )
    {
        //LOG_DEBUG_F( "%s: doReport = %d\n", __FUNCTION__, doReport );

        IIndividualHumanTyphoid* typhoid_individual = NULL;
        if( individual->QueryInterface( GET_IID( IIndividualHumanTyphoid ), (void**)&typhoid_individual ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualTyphoid", "IndividualHuman" );
        }

        float mc_weight = individual->GetMonteCarloWeight();
        int age_bin = (int)floor( (std::min)((double) MAX_AGE-1, individual->GetAge()/(float)DAYSPERYEAR) );

        int gender = individual->GetGender() == Gender::MALE ? 0 : 1;

        bool isInfected = individual->IsInfected();
        bool isChronic = typhoid_individual->IsChronicCarrier();
        bool isSub = typhoid_individual->IsSubClinical();
        bool isAcute = typhoid_individual->IsAcute(); 
        bool isPrePatent = typhoid_individual->IsPrePatent(); 

        // We do the incidences throughout the reporting period
        if( isChronic )
        {
            chronic_inc[ gender ][ age_bin ] += mc_weight;
        }
        if( isSub )
        {
            subClinical_inc[ gender ][ age_bin ] += mc_weight;
        }
        if( isAcute )
        {
            acute_inc[ gender ][ age_bin ] += mc_weight;
        }
        if( isPrePatent )
        {
            prePatent_inc[ gender ][ age_bin ] += mc_weight;
        }
        //LOG_DEBUG_F( "Individual %d, infected = %d, isPrePatent = %d, isChronic = %d, isSub = %d, isAcute = %d\n", individual->GetSuid().data, isInfected, isChronic, isSub, isAcute );

        // We do the prevalences only in the snapshot timestep in which we are writing out the report.
        if( doReport )
        {
            bool isInfected = individual->IsInfected();
            bool isChronic = typhoid_individual->IsChronicCarrier( false );
            bool isSub = typhoid_individual->IsSubClinical( false );
            bool isAcute = typhoid_individual->IsAcute( false );
            //bool isNewlyInfected = false;
            
            population[ gender ][ age_bin ] += mc_weight;

            if( isInfected )
            {
                infected[ gender ][ age_bin ] += mc_weight;
            }

            if( isChronic )
            {
                chronic[ gender ][ age_bin ] += mc_weight;
            }
            if( isSub )
            {
                subClinical[ gender ][ age_bin ] += mc_weight;
            }
            if( isAcute )
            {
                acute[ gender ][ age_bin ] += mc_weight;
            }
            if( isPrePatent )
            {
                prePatent[ gender ][ age_bin ] += mc_weight;
            }
            LOG_DEBUG_F( "Individual %d, infected = %d, isPrePatent = %d, isChronic = %d, isSub = %d, isAcute = %d\n", individual->GetSuid().data, isInfected, isPrePatent, isChronic, isSub, isAcute );
        }
    }

    bool ReportTyphoidByAgeAndGender::notifyOnEvent( IIndividualHumanEventContext *context, const std::string& StateChange )
    {
        LOG_DEBUG_F( "Individual %d experienced event %s\n",
                     context->GetSuid().data,
                     StateChange.c_str()
                   );

        float mc_weight = context->GetMonteCarloWeight();
        int gender = context->GetGender() == Gender::MALE ? 0 : 1;
        int age_bin = (int)floor( (std::min)((double) MAX_AGE-1, context->GetAge()/(float)DAYSPERYEAR) );

        if( StateChange == "NewInfectionEvent" )
        {
            newly_infected[ gender ][ age_bin ] += mc_weight;
        }
 
        return true;
    }
}

