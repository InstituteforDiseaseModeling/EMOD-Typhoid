/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReportHumanMigrationTracking.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "NodeEventContext.h"
#include "Individual.h"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Module name for logging, CustomReport.json, and DLL GetType()
static const char * _module = "ReportHumanMigrationTracking";// <<< Name of this file

namespace Kernel
{
// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = { "*", nullptr };// <<< Types of simulation the report is to be used with

report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new ReportHumanMigrationTracking()); // <<< Report to create
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
// --- ReportHumanMigrationTracking Methods
// ----------------------------------------

    ReportHumanMigrationTracking::ReportHumanMigrationTracking()
        : BaseTextReportEvents( "ReportHumanMigrationTracking.csv" )
        , m_EndTime(0.0)
        , m_MigrationDataMap()
    {
        // ------------------------------------------------------------------------------------------------
        // --- Since this report will be listening for events, it needs to increment its reference count
        // --- so that it is 1.  Other objects will be AddRef'ing and Release'ing this report/observer
        // --- so it needs to start with a refcount of 1.
        // ------------------------------------------------------------------------------------------------
        AddRef();
    }

    ReportHumanMigrationTracking::~ReportHumanMigrationTracking()
    {
    }

    bool ReportHumanMigrationTracking::Configure( const Configuration * inputJson )
    {

        bool ret = BaseTextReportEvents::Configure( inputJson );

        // Manually push required events into the eventTriggerList
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::Emigrating      ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::Immigrating     ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::NonDiseaseDeaths) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::DiseaseDeaths   ) );
        
        return ret;
    }

    void ReportHumanMigrationTracking::UpdateEventRegistration(  float currentTime, 
                                                       float dt, 
                                                       std::vector<INodeEventContext*>& rNodeEventContextList )
    {
        BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList );
    }


    std::string ReportHumanMigrationTracking::GetHeader() const
    {
        std::stringstream header ;
        header << "Time"          << ", "
               << "IndividualID"  << ", "
               << "AgeYears"      << ", "
               << "Gender"        << ", "
               << "IsAdult"       << ", "
               << "Home_NodeID"   << ", "
               << "From_NodeID"   << ", "
               << "To_NodeID"     << ", "
               << "MigrationType" << ", "
               << "Event"
               ;

        return header.str();
    }

    bool ReportHumanMigrationTracking::notifyOnEvent( IIndividualHumanEventContext *context, 
                                                      const std::string& eventStr )
    {
        IIndividualHuman* p_ih = nullptr;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHuman), (void**)&p_ih) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHuman", "IIndividualHumanEventContext");
        }
        IMigrate * im = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IMigrate), (void**)&im) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IMigrate", "IIndividualHumanEventContext");
        }

        ISimulationContext* p_sim = context->GetNodeEventContext()->GetNodeContext()->GetParent();

        float time = context->GetNodeEventContext()->GetTime().time ;
        long individual_id = context->GetSuid().data ;
        float age_years = p_ih->GetAge() / DAYSPERYEAR ;
        char is_adult = p_ih->IsAdult() ? 'T' : 'F';
        char gender = (p_ih->GetGender() == 0) ? 'M' : 'F' ;
        uint32_t home_node_id = p_sim->GetNodeExternalID( p_ih->GetHomeNodeId() ) ;
        uint32_t from_node_id = p_sim->GetNodeExternalID( context->GetNodeEventContext()->GetId() ) ;
        uint32_t to_node_id = from_node_id ;
        std::string mig_type_str = "local" ;
        bool is_emigrating  = (eventStr == IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::Emigrating  ));
        bool is_immigrating = (eventStr == IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::Immigrating ));

        if( is_immigrating )
        {
            if( p_ih->AtHome() )
            {
                if( m_MigrationDataMap.count( individual_id ) == 0 )
                {
                    std::ostringstream msg ;
                    msg << "Individual=" << individual_id << " is not in the map yet." ;
                    throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str());
                }
                m_MigrationDataMap[ individual_id ].node_id = to_node_id ;
            }
        }
        else if( is_emigrating )
        {
            to_node_id =  p_sim->GetNodeExternalID( im->GetMigrationDestination() ) ;

            int mig_type = im->GetMigrationType();
            if( p_ih->IsOnFamilyTrip() )
                mig_type_str = "family" ;
            else if( mig_type == MigrationType::LOCAL_MIGRATION )
                mig_type_str = "local" ;
            else if( mig_type == MigrationType::AIR_MIGRATION )
                mig_type_str = "air" ;
            else if( mig_type == MigrationType::REGIONAL_MIGRATION )
                mig_type_str = "regional" ;
            else if( mig_type == MigrationType::SEA_MIGRATION )
                mig_type_str = "sea" ;
            else if( mig_type == MigrationType::INTERVENTION_MIGRATION )
                mig_type_str = "intervention" ;
            else
                release_assert( false );

            // only keep track if the person is on the core that their home node is
            if( p_sim->GetNodeRank( p_ih->GetHomeNodeId() ) == EnvPtr->MPI.Rank )
            {
                if( m_MigrationDataMap.count( individual_id ) == 0 )
                {
                    std::ostringstream msg ;
                    msg << "Individual=" << individual_id << " is not in the map yet." ;
                    throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str());
                }
                m_MigrationDataMap[ individual_id ].age_years          = age_years ;
                m_MigrationDataMap[ individual_id ].is_adult           = p_ih->IsAdult() ;
                m_MigrationDataMap[ individual_id ].node_id            = to_node_id ;
                m_MigrationDataMap[ individual_id ].migration_type_str = mig_type_str ;
            }
        }
        else  // NonDiseaseDeaths || DiseaseDeaths
        {
            m_MigrationDataMap.erase( individual_id );
        }

        if( !is_immigrating )
        {
            GetOutputStream() << time
                       << "," << individual_id 
                       << "," << age_years 
                       << "," << gender 
                       << "," << is_adult 
                       << "," << home_node_id 
                       << "," << from_node_id 
                       << "," << to_node_id 
                       << "," << mig_type_str 
                       << "," << eventStr 
                       << endl;
        }
        return true;
    }

    void ReportHumanMigrationTracking::LogIndividualData( IIndividualHuman* individual ) 
    {
        long individual_id = individual->GetSuid().data ;

        ISimulationContext* p_sim = individual->GetEventContext()->GetNodeEventContext()->GetNodeContext()->GetParent();

        MigrationData md ;
        md.age_years          = individual->GetAge() / DAYSPERYEAR ;
        md.gender             = individual->GetGender();
        md.is_adult           = individual->IsAdult();
        md.home_node_id       = p_sim->GetNodeExternalID( individual->GetHomeNodeId() ) ;
        md.node_id            = p_sim->GetNodeExternalID( individual->GetEventContext()->GetNodeEventContext()->GetId() ) ;
        if( md.home_node_id == md.node_id )
        {
            md.migration_type_str = "home" ;
        }
        else
        {
            md.migration_type_str = "away" ;
        }

        if( m_MigrationDataMap.count( individual_id ) == 0 )
        {
            m_MigrationDataMap.insert( std::make_pair( individual_id, md ) );
        }
        else
        {
            m_MigrationDataMap[ individual_id ].age_years = md.age_years ;
            m_MigrationDataMap[ individual_id ].is_adult  = md.is_adult ;
            m_MigrationDataMap[ individual_id ].node_id   = md.node_id ;
        }
    }

    bool ReportHumanMigrationTracking::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return true ;
    }

    void ReportHumanMigrationTracking::EndTimestep( float currentTime, float dt )
    {
        m_EndTime = currentTime ;
        BaseTextReportEvents::EndTimestep( currentTime, dt );
    }

    void ReportHumanMigrationTracking::Reduce()
    {
        for( auto entry : m_MigrationDataMap )
        {
            std::string mig_type_str = entry.second.migration_type_str ;
            if( entry.second.node_id == entry.second.home_node_id )
            {
                mig_type_str = "home" ;
            }

            GetOutputStream() << m_EndTime
                       << "," << entry.first 
                       << "," << entry.second.age_years 
                       << "," << ((entry.second.gender == 0) ? 'M' : 'F') 
                       << "," << (entry.second.is_adult ? 'T' : 'F')
                       << "," << entry.second.home_node_id 
                       << "," << entry.second.node_id 
                       << "," << entry.second.node_id 
                       << "," << mig_type_str
                       << "," << "SimulationEnd"
                       << endl;
        }
        BaseTextReportEvents::EndTimestep( m_EndTime, 1.0 );
    }
}