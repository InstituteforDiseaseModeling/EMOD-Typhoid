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

#include "ReportRelationshipMigrationTracking.h"
#include "DllInterfaceHelper.h"
#include "FactorySupport.h"

#include "NodeEventContext.h"
#include "IIndividualHuman.h"
#include "VectorContexts.h"
#include "VectorPopulation.h"
#include "IMigrationInfo.h"
#include "IRelationship.h"
#include "IIndividualHumanSTI.h"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Module name for logging, CustomReport.json, and DLL GetType()
static const char * _module = "ReportRelationshipMigrationTracking";// <<< Name of this file

namespace Kernel
{
// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = { "STI_SIM", "HIV_SIM", nullptr };// <<< Types of simulation the report is to be used with

report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new ReportRelationshipMigrationTracking()); // <<< Report to create
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
// --- ReportRelationshipMigrationTracking Methods
// ----------------------------------------

    ReportRelationshipMigrationTracking::ReportRelationshipMigrationTracking()
        : BaseTextReportEvents( "ReportRelationshipMigrationTracking.csv" )
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

    ReportRelationshipMigrationTracking::~ReportRelationshipMigrationTracking()
    {
    }

    bool ReportRelationshipMigrationTracking::Configure( const Configuration * inputJson )
    {

        bool ret = BaseTextReportEvents::Configure( inputJson );

        // Manually push required events into the eventTriggerList
        //eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::Emigrating         ) );
        //eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::Immigrating        ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::STIPreEmigrating   ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::STIPostImmigrating ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::NonDiseaseDeaths   ) );
        eventTriggerList.push_back( IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::DiseaseDeaths      ) );
        
        return ret;
    }

    void ReportRelationshipMigrationTracking::UpdateEventRegistration(  float currentTime, 
                                                       float dt, 
                                                       std::vector<INodeEventContext*>& rNodeEventContextList )
    {
        BaseTextReportEvents::UpdateEventRegistration( currentTime, dt, rNodeEventContextList );
    }


    std::string ReportRelationshipMigrationTracking::GetHeader() const
    {
        std::stringstream header ;
        header << "Time"             << ", "
               << "IndividualID"     << ", "
               << "AgeYears"         << ", "
               << "Gender"           << ", "
               << "From_NodeID"      << ", "
               << "To_NodeID"        << ", "
               << "MigrationType"    << ", "
               << "Event"            << ","
               << "IsInfected"       << ", "
               << "Rel_ID"           << ", "
               << "NumCoitalActs"    << ", "
               << "IsDiscordant"     << ", "
               << "HasMigrated"      << ", "
               << "RelationshipType" << ", "
               << "RelationshipState"<< ", "
               << "PartnerID"        << ", "
               << "Male_NodeID"      << ", "
               << "Female_NodeID"
               ;

        return header.str();
    }

    bool ReportRelationshipMigrationTracking::notifyOnEvent( IIndividualHumanEventContext *context, 
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

        IIndividualHumanSTI* p_hsti = nullptr;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanSTI), (void**)&p_hsti) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanSTI", "IndividualHuman");
        }

        ISimulationContext* p_sim = context->GetNodeEventContext()->GetNodeContext()->GetParent();

        bool is_emigrating  = (eventStr == IndividualEventTriggerType::pairs::lookup_key( IndividualEventTriggerType::STIPreEmigrating  ));

        float time = context->GetNodeEventContext()->GetTime().time ;
        long individual_id = context->GetSuid().data ;
        float age_years = p_ih->GetAge() / DAYSPERYEAR ;
        char gender = (p_ih->GetGender() == 0) ? 'M' : 'F' ;
        uint32_t from_node_id = p_sim->GetNodeExternalID( context->GetNodeEventContext()->GetId() ) ;
        uint32_t to_node_id = 0;
        std::string mig_type_str = "N/A" ;
        if( is_emigrating )
        {
            to_node_id = p_sim->GetNodeExternalID( im->GetMigrationDestination() ) ;
            int mig_type = im->GetMigrationType();
            if( mig_type == MigrationType::LOCAL_MIGRATION )
                mig_type_str = "local" ;
            else if( mig_type == MigrationType::AIR_MIGRATION )
                mig_type_str = "air" ;
            else if( mig_type == MigrationType::REGIONAL_MIGRATION )
                mig_type_str = "regional" ;
            else if( mig_type == MigrationType::SEA_MIGRATION )
                mig_type_str = "sea" ;
            else
                release_assert( false );
        }

        bool is_infected = context->IsInfected();
        for( auto prel : p_hsti->GetRelationships() )
        {
            int rel_id = prel->GetSuid().data;
            int female_id = prel->GetFemalePartnerId().data;
            int male_id   = prel->GetMalePartnerId().data;
            int partner_id = 0;
            if( female_id == context->GetSuid().data )
            {
                partner_id = male_id;
            }
            else
            {
                partner_id = female_id;
            }
            unsigned int num_acts = prel->GetNumCoitalActs();
            bool is_discordant = prel->IsDiscordant();
            bool has_migrated = prel->HasMigrated();
            const char* rel_type_str  = RelationshipType::pairs::lookup_key( prel->GetType() );
            const char* rel_state_str = RelationshipState::pairs::lookup_key( prel->GetState() );
            unsigned int male_node_id = 0;
            if( prel->MalePartner() != nullptr )
            {
                male_node_id = prel->MalePartner()->GetNodeSuid().data;
            }
            unsigned int female_node_id = 0;
            if( prel->FemalePartner() != nullptr )
            {
                female_node_id = prel->FemalePartner()->GetNodeSuid().data;
            }

            GetOutputStream() << time
                       << "," << individual_id 
                       << "," << age_years 
                       << "," << gender 
                       << "," << from_node_id 
                       << "," << to_node_id 
                       << "," << mig_type_str 
                       << "," << eventStr 
                       << "," << is_infected 
                       << "," << rel_id 
                       << "," << num_acts 
                       << "," << is_discordant 
                       << "," << has_migrated 
                       << "," << rel_type_str 
                       << "," << rel_state_str 
                       << "," << partner_id 
                       << "," << male_node_id 
                       << "," << female_node_id 
                       << endl;
        }

        return true;
    }

    void ReportRelationshipMigrationTracking::LogIndividualData( IIndividualHuman* individual ) 
    {
    }

    bool ReportRelationshipMigrationTracking::IsCollectingIndividualData( float currentTime, float dt ) const
    {
        return false ;
    }

    void ReportRelationshipMigrationTracking::EndTimestep( float currentTime, float dt )
    {
        m_EndTime = currentTime ;
        BaseTextReportEvents::EndTimestep( currentTime, dt );
    }

    void ReportRelationshipMigrationTracking::Reduce()
    {
        BaseTextReportEvents::EndTimestep( m_EndTime, 1.0 );
    }
}