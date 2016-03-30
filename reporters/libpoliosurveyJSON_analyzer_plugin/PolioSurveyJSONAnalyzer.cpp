/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "StdAfx.h"
#include "PolioDefs.h"
#include "PolioSurveyJSONAnalyzer.h"

#include "BoostLibWrapper.h"
#include "Environment.h"
#include "Exceptions.h"
#include "FileSystem.h"
#include "IndividualEventContext.h"
#include "NodeEventContext.h"

#include "IndividualPolio.h"
#include "SusceptibilityPolio.h"
#include "InfectionPolio.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Module name for logging, CustomReport.json, and DLL GetType()
static const char * _module = "PolioSurveyJSONAnalyzer"; // <<< Name of this file

namespace Kernel
{
// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = {"POLIO_SIM", nullptr}; // <<< Types of simulation the report is to be used with

Kernel::report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new PolioSurveyJSONAnalyzer()); // <<< Report to create
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

// ---------------------------
// --- PolioPatient Methods
// ---------------------------

    PolioPatient::PolioPatient(int id_, float age_)
        : id(id_)
        , initial_age(age_)
    {
    }

    PolioPatient::~PolioPatient()
    {
    }

    void PolioPatient::JSerialize( IJsonObjectAdapter* root, JSerializer* helper )
    {
        LOG_DEBUG("Serializing PolioPatient\n");
        root->BeginObject();

        LOG_DEBUG("Inserting simple variables\n");
        root->Insert("id", id);
        root->Insert("initial_age", initial_age);

        //LOG_DEBUG("Inserting array variables\n");
        SerializeChannel("mucosal_Nab",        mucosal_Nab,       root, helper);
        SerializeChannel("humoral_Nab",        humoral_Nab,       root, helper);
        SerializeChannel("shedding_titer",     shedding_titer,    root, helper);
        SerializeChannel("mucosal_memory_NAb", mucosalmemory_Nab, root, helper);
        SerializeChannel("humoral_memory_NAb", humoralmemory_Nab, root, helper);
        SerializeChannel("serum_NAb",          maternalserumNAb,  root, helper);

        root->EndObject();
    }

    void PolioPatient::SerializeChannel(
        std::string channel_name,
        serotype_channel_data_t &channel_data,
        IJsonObjectAdapter* root,
        JSerializer* helper
    )
    {
        root->Insert(channel_name.c_str());
        root->BeginObject();
        for (auto &serotype_channel_data : channel_data)
        {
            serotype_name_t type = serotype_channel_data.first;
            channel_data_t data  = serotype_channel_data.second;
            root->Insert(type.c_str());
            root->BeginArray();
            helper->JSerialize(data, root);
            root->EndArray();
        }
        root->EndObject();
    }

// ----------------------------------------
// --- PolioSurveyJSONAnalyzer Methods
// ----------------------------------------

    PolioSurveyJSONAnalyzer::PolioSurveyJSONAnalyzer()
        : BaseEventReportIntervalOutput( _module ) 
        , m_patient_map()
        , m_node_id(-1)
        , m_pst_map()
        , m_pvt_map()
    {
        LOG_DEBUG( "CTOR\n" );
        // enum PolioSerotypes {PV1 = 0, PV2 = 1, PV3 = 2};
        // Better solution: make these evil-enums
        m_pst_map[PolioSerotypes::PV1]="PV1";
        m_pst_map[PolioSerotypes::PV2]="PV2";
        m_pst_map[PolioSerotypes::PV3]="PV3"; 

        // enum PolioVirusTypes {WPV1 = 0, WPV2 = 1, WPV3 = 2, VRPV1 = 3, VRPV2 = 4, VRPV3 = 5};
        m_pvt_map[PolioVirusTypes::WPV1]  = "WPV1";
        m_pvt_map[PolioVirusTypes::WPV2] =  "WPV2";
        m_pvt_map[PolioVirusTypes::WPV3] =  "WPV3"; 
        m_pvt_map[PolioVirusTypes::VRPV1] = "VRPV1";
        m_pvt_map[PolioVirusTypes::VRPV2] = "VRPV2";
        m_pvt_map[PolioVirusTypes::VRPV3] = "VRPV3"; 
    }

    PolioSurveyJSONAnalyzer::~PolioSurveyJSONAnalyzer()
    {
        LOG_DEBUG( "DTOR\n" );
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!! Commenting out the code to delete all the patients because it can take
        // !!! a long time to delete them.  If we are doing this, we are trying to delete
        // !!! the report and exit the simulation.  Hurry up and exit.
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //ClearOutputData();
    }

    bool PolioSurveyJSONAnalyzer::notifyOnEvent( IIndividualHumanEventContext *context, const std::string& StateChange )
    {
        if( HaveUnregisteredAllEvents() )
        {
            return false ;
        }
        LOG_DEBUG_F( "PolioSurveyJSONAnalyzer notified of event by %d-year old individual.\n", (int) (context->GetAge() / DAYSPERYEAR) );

        if( m_node_id == -1 )
        {
			m_node_id = context->GetNodeEventContext()->GetId().data;
        }
        LOG_DEBUG_F( "Node ID = %d.\n", m_node_id);

        // individual context for suid
        IIndividualHumanContext * iindividual = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanContext), (void**)&iindividual) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanContext", "IIndividualHumanEventContext");
        }

        int id           = iindividual->GetSuid().data;
        double mc_weight = context->GetMonteCarloWeight();
        double age       = context->GetAge();

        // get the correct existing patient or insert a new one
        PolioPatient* patient = NULL;
        patient_map_t::const_iterator it = m_patient_map.find(id);
        if ( it == m_patient_map.end() )
        {
            patient = new PolioPatient(id, age);
            m_patient_map.insert( std::make_pair(id, patient) );
        }
        else
        {
            patient = it->second;
        }

        // get polio contexts
        IIndividualHumanPolio * iip = NULL;
        if (s_OK != iindividual->QueryInterface(GET_IID(IIndividualHumanPolio), (void**)&iip) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "iindividual", "IIndividualHumanPolio", "IIndividualHumanContext");
        }
        auto susc_polio = iip->GetSusceptibilityReporting();

        // Grabbing daily values from the ISusceptibilityPolio
        float mucAb[N_POLIO_SEROTYPES] = {0.0, 0.0, 0.0};
        susc_polio->GetMucosalNAb(mucAb);
        float humAb[N_POLIO_SEROTYPES] = {0.0, 0.0, 0.0};
        susc_polio->GetHumoralNAb(humAb);
        float mucmemNAB[N_POLIO_SEROTYPES] = {0.0 ,0.0, 0.0};
        susc_polio->GetMucosalMemoryNAb(mucmemNAB);
        float hummemNAb[N_POLIO_SEROTYPES] = {0.0, 0.0, 0.0};
        susc_polio->GetHumoralMemoryNAb(hummemNAb);
        float serumNab[N_POLIO_SEROTYPES] = {0.0, 0.0, 0.0};
        susc_polio->GetMaternalSerumNAb(serumNab);

        for (int idx=0; idx<N_POLIO_SEROTYPES; idx++)
        {
            patient->mucosal_Nab[       m_pst_map[idx] ].push_back( mucAb[idx]     );
            patient->humoral_Nab[       m_pst_map[idx] ].push_back( humAb[idx]     );
            patient->humoralmemory_Nab[ m_pst_map[idx] ].push_back( hummemNAb[idx] );
            patient->mucosalmemory_Nab[ m_pst_map[idx] ].push_back( mucmemNAB[idx] );
            patient->maternalserumNAb[  m_pst_map[idx] ].push_back( serumNab[idx]  );
        }

        // Going through the list of IInfectionPolioReportable objects (i.e. constant interfaces of InfectionPolio)
        const infection_polio_reportable_list_t *iprl = iip->GetInfectionReportables(false);
        std::map<int,float> infectiousness_by_antigen;
        for ( auto ipr : *iprl )
        {
            int antigenID = ipr->GetAntigenID();
            infectiousness_by_antigen[antigenID] = ipr->GetInfectiousness();
        }

        for (int idx=0; idx<N_POLIO_VIRUS_TYPES; idx++)
        {
            auto inf_it = infectiousness_by_antigen.find(idx);
            if ( inf_it != infectiousness_by_antigen.end() )
            {
                patient->shedding_titer[ m_pvt_map[idx] ].push_back( inf_it->second );
            }
            else
            {
                patient->shedding_titer[ m_pvt_map[idx] ].push_back( 0.0f );
            }
        }

        return true;
    }

    void PolioSurveyJSONAnalyzer::WriteOutput( float currentTime )
    {
        // Open output file
        ofstream ofs;
        std::ostringstream output_file_name;
        output_file_name << GetBaseOutputFilename();
        output_file_name << "_" << m_node_id;
		if ( (currentTime >= 0.0f) && (m_report_count > 0) )
        {
            output_file_name << "_" << (int)currentTime;
        }
        output_file_name << ".json";
        LOG_INFO_F( "Writing file: %s\n", output_file_name.str().c_str() );
        ofs.open( FileSystem::Concat( EnvPtr->OutputPath, output_file_name.str() ) );
        if (!ofs.is_open())
        {
            LOG_ERR("Failed to open output file for serialization.\n");
            throw Kernel::FileNotFoundException( __FILE__, __LINE__, __FUNCTION__, output_file_name.str().c_str() );
        }

        // Accumulate array of patients as JSON
        int counter = 0;
        JSerializer js;
        LOG_DEBUG("Creating JSON object adaptor.\n");
        IJsonObjectAdapter* pIJsonObj = CreateJsonObjAdapter();
        LOG_DEBUG("Creating new JSON writer.\n");
        pIJsonObj->CreateNewWriter();
        LOG_DEBUG("Beginning polio patient array.\n");
        pIJsonObj->BeginObject();
        pIJsonObj->Insert("ntsteps", m_reporting_interval);
        pIJsonObj->Insert("patient_array");
        pIJsonObj->BeginArray();
        for(auto &id_patient_pair: m_patient_map)
        {
            PolioPatient* patient = id_patient_pair.second;
            LOG_DEBUG_F("Serializing patient %d\n", counter);
            patient->JSerialize(pIJsonObj, &js);
            LOG_DEBUG_F("Finished serializing patient %d\n", counter);
            counter++;
        }
        pIJsonObj->EndArray();
        pIJsonObj->EndObject();

        // Write output to file
        // GetFormattedOutput() could be used for a smaller but less human readable file
        char* sHumans;
        //js.GetFormattedOutput( pIJsonObj, sHumans );
        js.GetPrettyFormattedOutput(pIJsonObj, sHumans);
        if (sHumans)
        {
            ofs << sHumans << endl;
            delete sHumans ;
            sHumans = nullptr ;
            LOG_DEBUG("Done inserting\n");
        }
        else
        {
            LOG_ERR("Failed to get prettyHumans\n");
            throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, output_file_name.str().c_str() );
        }
        if (ofs.is_open())
        {
            ofs.close();
            LOG_DEBUG("Done writing\n");
        }
        pIJsonObj->FinishWriter();
        delete pIJsonObj ;
    }

    void PolioSurveyJSONAnalyzer::ClearOutputData()
    {
        for( auto &patient: m_patient_map )
        {
            delete patient.second;
        }
        m_patient_map.clear();
    }
}
