/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "Report_HIV_WHO2015.h"
#include "DllInterfaceHelper.h"

#include "NodeEventContext.h"
#include "Individual.h"
#include "IIndividualHumanHIV.h"
#include "IHIVInterventionsContainer.h"
#include "SusceptibilityHIV.h"

// TODO: 
// --> Start_Year
// --> Every 6 months
// --> Strings for CD4 stage and care stage
// --> Functions computing cd4_stage and care_stage
// --> Function for counter reset

// BASE_YEAR is temporary until Year() is fixed!
#define BASE_YEAR (1960.5f)
#define FIFTEEN_YEARS (15.0f * DAYSPERYEAR)
#define SIX_MONTHS (0.5f * DAYSPERYEAR)

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Module name for logging, CustomReport.json, and DLL GetType()
static const char * _module = "Report_HIV_WHO2015";// <<< Name of this file

namespace Kernel
{
// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = { "HIV_SIM", nullptr };// <<< Types of simulation the report is to be used with

report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new Report_HIV_WHO2015()); // <<< Report to create
};

DllInterfaceHelper DLL_HELPER( _module, _sim_types, rif );

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// ------------------------------
// --- DLL Interface Methods
// ---
// --- The DTK will use these methods to establish communication with the DLL.
// ------------------------------

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

DTK_DLLEXPORT char* __cdecl
GetEModuleVersion(char* sVer, const Environment * pEnv)
{
    return DLL_HELPER.GetEModuleVersion( sVer, pEnv );
}

DTK_DLLEXPORT void __cdecl
GetSupportedSimTypes(char* simTypes[])
{
    DLL_HELPER.GetSupportedSimTypes( simTypes );
}

DTK_DLLEXPORT const char * __cdecl
GetType()
{
    return DLL_HELPER.GetType();
}

DTK_DLLEXPORT void __cdecl
GetReportInstantiator( Kernel::report_instantiator_function_t* pif )
{
    DLL_HELPER.GetReportInstantiator( pif );
}

#ifdef __cplusplus
}
#endif

// ----------------------------------------
// --- Report_HIV_WHO2015 Methods
// ----------------------------------------

    Report_HIV_WHO2015::Report_HIV_WHO2015()
        : BaseTextReportEvents( "Report_HIV_WHO2015.csv" )
        , report_hiv_half_period( DAYSPERYEAR / 2.0f )
        , next_report_time(report_hiv_half_period)
        , doReport( false )
        , startYear(0.0)
        , stopYear(FLT_MAX)
        , is_collecting_data(false)
    {
        // ------------------------------------------------------------------------------------------------
        // --- Since this report will be listening for events, it needs to increment its reference count
        // --- so that it is 1.  Other objects will be AddRef'ing and Release'ing this report/observer
        // --- so it needs to start with a refcount of 1.
        // ------------------------------------------------------------------------------------------------
        AddRef();

        // Add events to event trigger list

        // Call a reset function
        ZERO_ARRAY( Population );
        ZERO_ARRAY( NonDiseaseDeaths );
        ZERO_ARRAY( DiseaseDeaths );
        ZERO_ARRAY( ART_Initiations );
        New_Infections      = 0;
        ZERO_ARRAY(New_Diagnoses);
        ZERO_ARRAY(PreART_Dropouts);
        ART_Dropouts        = 0;
    }

    Report_HIV_WHO2015::~Report_HIV_WHO2015()
    {
    }

    bool Report_HIV_WHO2015::Configure( const Configuration * inputJson )
    {
        initConfigTypeMap( "Start_Year", &startYear, "Year to start collecting data for Report_HIV_WHO2015.csv", 0.0f, FLT_MAX, 0.0f    );
        initConfigTypeMap( "Stop_Year",  &stopYear,  "Year to stop collecting data for Report_HIV_WHO2015.csv",  0.0f, FLT_MAX, FLT_MAX );

        bool ret = BaseTextReportEvents::Configure( inputJson );

        if( ret ) {
            if( startYear < BASE_YEAR ) //IdmDateTime::_base_year
            {
                startYear = BASE_YEAR;  //IdmDateTime::_base_year ;
            }
            if( startYear >= stopYear )
            {
                 throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Start_Year", startYear, "Stop_Year", stopYear );
            }
        }

        // Manually push required events into the eventTriggerList
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::HIVNewlyDiagnosed ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::StartedART ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::StoppedART ) );
        eventTriggerList.push_back( "HCTUptakePostDebut9" );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::NewInfectionEvent ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::DiseaseDeaths ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::NonDiseaseDeaths ) );
        
        return ret;
    }

    void Report_HIV_WHO2015::UpdateEventRegistration(  float currentTime, 
                                                       float dt, 
                                                       std::vector<INodeEventContext*>& rNodeEventContextList )
    {
        // not enforcing simulation to be not null in constructor so one can create schema with it null

        release_assert( !rNodeEventContextList.empty() );

        // BASE_YEAR is TEMPORARY HERE!!!
        float current_year = BASE_YEAR + rNodeEventContextList.front()->GetTime().Year();

        if( !is_collecting_data && (startYear <= current_year) && (current_year < stopYear) )
        {
            BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList );
            is_collecting_data = true ;

            // ------------------------------------------------------------------------
            // --- The idea here is to ensure that as we increase the startYear from the 
            // --- base_year we get the same report times as when startYear = base_year
            // --- The difference between start and base year gives us the number of days
            // --- into the simulation that we think we should be.  It ignores issues with
            // --- the size of dt not ending exactly on integer years.  We subtract the
            // --- dt/2 to deal with rounding errors.  For example, if the half period was 182.5,
            // --- start_year == _base_year, and dt = 30, then next_report_time = 167.5 and
            // --- data would be collected at 180.  However, for the next update
            // --- next_report_time would be 350 and the update would occur at 360.
            // ------------------------------------------------------------------------
            next_report_time = DAYSPERYEAR*(startYear - BASE_YEAR) + report_hiv_half_period - dt / 2.0f ;
        }
        else if( is_collecting_data && (current_year >= stopYear) )
        {
            UnregisterAllNodes();
            is_collecting_data = false ;
        }

        if( is_collecting_data )
        {
            // Figure out when to set doReport to true.  doReport is true for those
            // timesteps where we take a snapshot, i.e., as a function of the
            // half-year offset and full-year periodicity.
            doReport = false;

            if( currentTime >= next_report_time ) 
            {
                next_report_time += report_hiv_half_period;

                LOG_DEBUG_F( "Setting doReport to true .\n" );
                doReport = true;
            }
        }
    }

    CD4_Stage::Enum Report_HIV_WHO2015::ComputeCD4Stage(IIndividualHumanEventContext *context)
    {
        CD4_Stage::Enum cd4_stage = CD4_Stage::HIV_NEGATIVE;

        IIndividualHumanHIV* hiv_individual = NULL;
        if( context->QueryInterface( GET_IID( IIndividualHumanHIV ), (void**)&hiv_individual ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman" );
        }


        // CD4 Stage
        if( hiv_individual->HasHIV() ) {
            float cd4 = hiv_individual->GetHIVSusceptibility()->GetCD4count();
            if( cd4 < 200 )
                cd4_stage = CD4_Stage::CD4_UNDER_200;
            else if( cd4 < 350 )
                cd4_stage = CD4_Stage::CD4_200_TO_350;
            else if( cd4 < 500 )
                cd4_stage = CD4_Stage::CD4_350_TO_500;
            else
                cd4_stage = CD4_Stage::CD4_ABOVE_500;
        }

        return cd4_stage;
    }

    Care_Stage::Enum Report_HIV_WHO2015::ComputeCareStage(IIndividualHumanEventContext *context)
    {
        Care_Stage::Enum care_stage = Care_Stage::Enum::NA;

        IIndividualHumanHIV* hiv_individual = NULL;
        if( context->QueryInterface( GET_IID( IIndividualHumanHIV ), (void**)&hiv_individual ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHIV", "IndividualHuman" );
        }

        if( !hiv_individual->HasHIV() )
            return care_stage;     // Care_Stage::Enum::NA

        IHIVMedicalHistory * med_parent = nullptr;
        if (context->GetInterventionsContext()->QueryInterface(GET_IID(IHIVMedicalHistory), (void**)&med_parent) != s_OK)
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context->GetInterventionsContext()", "IHIVMedicalChart", "IndividualHuman" );
        }

        bool isOnART = hiv_individual->GetHIVInterventionsContainer()->OnArtQuery();

        bool everBledForCD4 = med_parent->TimeOfMostRecentCD4() > 0;

        // Care Stage
        if( isOnART ) { // Currently on ART
            if( hiv_individual->GetHIVInterventionsContainer()->GetDurationSinceLastStartingART() > SIX_MONTHS )
                care_stage = Care_Stage::Enum::ON_ART_MORE_THAN_SIX_MONTHS;
            else
                care_stage = Care_Stage::Enum::ON_ART_SIX_OR_FEWER_MONTHS;
        } else if( med_parent->EverBeenOnART() )
            care_stage = Care_Stage::Enum::INITIATED_ART_BUT_NO_LONGER_ON_ART;
        else if( med_parent->EverTestedHIVPositive() ) { // Never been on ART, in care?

            // "In care" if cascade_state is in ["ARTStaging", "LinkingToART", "LinkingToPreART", "OnART", "OnPreART"]

            IHIVCascadeOfCare *ihcc = nullptr;
            if ( s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IHIVCascadeOfCare), (void **)&ihcc) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context->GetInterventionsContext()", "IHIVCascadeOfCare", "IndividualHuman" );
            }

            std::string currentState = ihcc->getCascadeState();

            if( currentState.compare("ARTStaging") == 0 || 
                currentState.compare("LinkingToART") == 0 || 
                currentState.compare("LinkingToPreART") == 0 ||
                currentState.compare("OnART") == 0 ||
                currentState.compare("OnPreART") == 0 )
            {  // In care
                care_stage = Care_Stage::Enum::IN_CARE_BUT_NEVER_INITIATED_ART;
            } else {
                //care_stage = Care_Stage::Enum::DIAGNOSED_BUT_NOT_IN_CARE;
                if( everBledForCD4 )
                    care_stage = Care_Stage::Enum::DIAGNOSED_AND_LINKED_BUT_NOT_IN_CARE;
                else
                    care_stage = Care_Stage::Enum::DIAGNOSED_BUT_NEVER_LINKED;
            }
        } else
            care_stage = Care_Stage::Enum::NOT_DIAGNOSED;

        return care_stage;
    }

    std::string Report_HIV_WHO2015::GetHeader() const
    {
        std::stringstream header ;
        header << "Year"             << ", "
               << "NodeID"           << ", "
               << "CD4_Stage"        << ", "
               << "Care_Stage"       << ", "
               << "Population"       << ", "
               << "DiseaseDeaths"    << ", "
               << "NonDiseaseDeaths" << ", "
               << "ART_Initiations"  << ", "
               << "New_Infections"   << ", "
               << "New_Diagnoses"    << ", "
               << "PreART_Dropouts"  << ", "
               << "ART_Dropouts";

        return header.str();
    }

    void Report_HIV_WHO2015::LogNodeData( Kernel::INodeContext* pNC )
    {
        if( (is_collecting_data == false) || (doReport == false) )
        {
            return;
        }
        LOG_DEBUG_F( "%s: doReport = %d\n", __FUNCTION__, doReport );

        // BASE_YEAR is TEMPORARY HERE!
        float year = BASE_YEAR + pNC->GetTime().Year();
        int nodeId = pNC->GetExternalID();

        for( int cd4stage_idx = 0; cd4stage_idx < CD4_Stage::Enum::COUNT; cd4stage_idx++ ) 
        {
            for( int carestage_idx = 0; carestage_idx < Care_Stage::Enum::COUNT; carestage_idx++ )
            {
                GetOutputStream() << year
                                  << "," << nodeId 
                                  << "," << CD4_Stage::pairs::lookup_key(cd4stage_idx)
                                  << "," << Care_Stage::pairs::lookup_key(carestage_idx)
                                  << "," << Population[cd4stage_idx][carestage_idx]
                                  << "," << DiseaseDeaths[cd4stage_idx][carestage_idx]
                                  << "," << NonDiseaseDeaths[cd4stage_idx][carestage_idx]
                                  << "," << (carestage_idx == (int)Care_Stage::Enum::NA ? ART_Initiations[cd4stage_idx] : 0.0f)
                                  << "," << (cd4stage_idx == (int)CD4_Stage::CD4_ABOVE_500 && carestage_idx == (int)Care_Stage::Enum::NA ? New_Infections : 0.0f)
                                  << "," << (carestage_idx == (int)Care_Stage::Enum::DIAGNOSED_BUT_NEVER_LINKED ? New_Diagnoses[cd4stage_idx] : 0.0f)
                                  << "," << (carestage_idx == (int)Care_Stage::Enum::DIAGNOSED_AND_LINKED_BUT_NOT_IN_CARE ? PreART_Dropouts[cd4stage_idx] : 0.0f)
                                  << "," << (cd4stage_idx == (int)CD4_Stage::CD4_ABOVE_500 && carestage_idx == (int)Care_Stage::Enum::INITIATED_ART_BUT_NO_LONGER_ON_ART ? ART_Dropouts : 0.0f)
                                  << endl;
            }
        }

        // Call a reset function
        ZERO_ARRAY( Population );
        ZERO_ARRAY( DiseaseDeaths );
        ZERO_ARRAY( NonDiseaseDeaths );
        ZERO_ARRAY( ART_Initiations );
        New_Infections      = 0;
        ZERO_ARRAY(New_Diagnoses);
        ZERO_ARRAY(PreART_Dropouts);
        ART_Dropouts        = 0;
    }

    bool Report_HIV_WHO2015::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return is_collecting_data && doReport; 
    }

    void Report_HIV_WHO2015::LogIndividualData( Kernel::IndividualHuman* individual )
    {
        if( individual->GetAge() < FIFTEEN_YEARS )
            return;

        float mc_weight = individual->GetMonteCarloWeight();

        CD4_Stage::Enum cd4_stage = ComputeCD4Stage(individual);
        Care_Stage::Enum care_stage = ComputeCareStage(individual);

        // Now increment prevalence counters
        Population[cd4_stage][care_stage] += mc_weight;
    }

    bool Report_HIV_WHO2015::notifyOnEvent( IIndividualHumanEventContext *context, 
                                            const std::string& StateChange )
    {
        if( context->GetAge() < FIFTEEN_YEARS )
            return true;

        // iindividual context for suid
        IIndividualHumanContext * iindividual = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanContext), (void**)&iindividual) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanContext", "IIndividualHumanEventContext");
        }

        float mc_weight = context->GetMonteCarloWeight();

        CD4_Stage::Enum cd4_stage = ComputeCD4Stage(context);
        Care_Stage::Enum care_stage = ComputeCareStage(context);

        if( StateChange == "HIVNewlyDiagnosed" )
        {
            New_Diagnoses[cd4_stage] += mc_weight;
        }
        else if( StateChange == "StartedART" )
        {
            ART_Initiations[cd4_stage] += mc_weight;
        }
        else if( StateChange == "StoppedART" )
        {
            ART_Dropouts += mc_weight;
        }
        else if( StateChange == "HCTUptakePostDebut9" )   // HCTUptakePostDebut9 is broadcast on PreART dropout
        {
            PreART_Dropouts[cd4_stage] += mc_weight;
        }
        else if( StateChange == "NewInfectionEvent" )
        {
            New_Infections += mc_weight;
        }
        else if( StateChange == "DiseaseDeaths" )
        {
            DiseaseDeaths[cd4_stage][care_stage] += mc_weight;
        }
        else if( StateChange == "NonDiseaseDeaths" )
        {
            NonDiseaseDeaths[cd4_stage][care_stage] += mc_weight;
        }

        return true;
    }

}