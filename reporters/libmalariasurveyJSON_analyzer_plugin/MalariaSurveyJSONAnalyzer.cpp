/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "StdAfx.h"
#include "MalariaSurveyJSONAnalyzer.h"

#include "FileSystem.h"
#include "Environment.h"
#include "Exceptions.h"
#include "IndividualMalaria.h"
#include "SusceptibilityMalaria.h"

#include "DllInterfaceHelper.h"
#include "DllDefs.h"
#include "ProgVersion.h"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! CREATING NEW REPORTS
// !!! If you are creating a new report by copying this one, you will need to modify 
// !!! the values below indicated by "<<<"

// Module name for logging, CustomReport.json, and DLL GetType()
static const char * _module = "MalariaSurveyJSONAnalyzer"; // <<< Name of this file

namespace Kernel
{
// You can put 0 or more valid Sim types into _sim_types but has to end with nullptr.
// "*" can be used if it applies to all simulation types.
static const char * _sim_types[] = {"MALARIA_SIM", nullptr}; // <<< Types of simulation the report is to be used with

Kernel::report_instantiator_function_t rif = []()
{
    return (Kernel::IReport*)(new MalariaSurveyJSONAnalyzer()); // <<< Report to create
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
// --- MalariaPatent Methods
// ---------------------------

    MalariaPatient::MalariaPatient(int id_, float age_, float local_birthday_)
        : id(id_)
        , initial_age(age_)
        , local_birthday(local_birthday_)
    {
    }

    MalariaPatient::~MalariaPatient()
    {
    }

    void MalariaPatient::JSerialize( IJsonObjectAdapter* root, JSerializer* helper )
    {
        LOG_DEBUG("Serializing MalariaPatient\n");
        root->BeginObject();

        LOG_DEBUG("Inserting simple variables\n");
        root->Insert("id", id);
        root->Insert("initial_age", initial_age);
        root->Insert("local_birthday", local_birthday);

        root->Insert("strain_ids");
        root->BeginArray();
        for (auto& inner : strain_ids)
        {
            // TODO: add helper function for serializing pair<int,int> ??
            root->BeginArray();
            root->Add(inner.first);
            root->Add(inner.second);
            root->EndArray();
        }
        root->EndArray();

        LOG_DEBUG("Inserting array variables\n");
        SerializeChannel("true_asexual_parasites", true_asexual_density, root, helper);
        SerializeChannel("true_gametocytes", true_gametocyte_density, root, helper);
        SerializeChannel("asexual_parasites", asexual_parasite_density, root, helper);
        SerializeChannel("gametocytes", gametocyte_density, root, helper);
        SerializeChannel("infectiousness", infectiousness, root, helper);
        SerializeChannel("pos_asexual_fields", pos_asexual_fields, root, helper);
        SerializeChannel("pos_gametocyte_fields", pos_gametocyte_fields, root, helper);
        SerializeChannel("temps", fever, root, helper);
        SerializeChannel("rdt", rdt, root, helper);

        root->EndObject();
    }

    void MalariaPatient::SerializeChannel( std::string channel_name, std::vector<float> &channel_data,
                                           IJsonObjectAdapter* root, JSerializer* helper )
    {
        root->Insert(channel_name.c_str());
        root->BeginArray();
        helper->JSerialize(channel_data, root);
        root->EndArray();
    }

// ----------------------------------------
// --- MalariaSurveyJSONAnalyzer Methods
// ----------------------------------------

    MalariaSurveyJSONAnalyzer::MalariaSurveyJSONAnalyzer() 
        : BaseEventReportIntervalOutput( _module )
        , m_patient_map()
    {
        LOG_DEBUG( "CTOR\n" );
    }

    MalariaSurveyJSONAnalyzer::~MalariaSurveyJSONAnalyzer()
    {
        LOG_DEBUG( "DTOR\n" );
        if( m_patient_map.size() > 0 )
        {
            WriteOutput( -999.0f );
        }
        ClearOutputData();
    }

    //    initConfigTypeMap("Coverage",           &m_coverage, "Coverage Fraction", 0, 1, 1);
    //    initConfigTypeMap("Include_Births",     &m_include_births,     "Include new births in survey", false);

    // N.B. implicit in the current architecture, the previously configurable parameters are set as follows:
    //      Coverage=1  and Include_Births=true

    bool MalariaSurveyJSONAnalyzer::notifyOnEvent( IIndividualHumanEventContext *context, const std::string& StateChange )
    {
        if( HaveUnregisteredAllEvents() )
        {
            return false ;
        }

        LOG_DEBUG_F( "MalariaSurveyAnalyzer notified of event by %d-year old individual.\n", (int) (context->GetAge() / DAYSPERYEAR) );

        // individual context for suid
        IIndividualHumanContext * iindividual = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanContext), (void**)&iindividual) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanContext", "IIndividualHumanEventContext");
        }

        int id           = iindividual->GetSuid().data;
        double mc_weight = context->GetMonteCarloWeight();
        double age       = context->GetAge();

        // get malaria contexts
        IMalariaHumanContext * individual_malaria = NULL;
        if (s_OK != context->QueryInterface(GET_IID(IMalariaHumanContext), (void**)&individual_malaria) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IMalariaHumanContext", "IIndividualHumanEventContext");
        }
        IMalariaSusceptibility* susceptibility_malaria = individual_malaria->GetMalariaSusceptibilityContext();

        // get the correct existing patient or insert a new one
        MalariaPatient* patient = NULL;
        patient_map_t::const_iterator it = m_patient_map.find(id);
        if ( it == m_patient_map.end() )
        {
            patient = new MalariaPatient(id, age, m_interval_timer-age); // birthday relative to current interval (i.e. output file)
            m_patient_map.insert( std::make_pair(id, patient) );

            patient->strain_ids = individual_malaria->GetInfectingStrainIds();
        }
        else
        {
            patient = it->second;
        }

        // Push back today's disease variables
        float max_fever = susceptibility_malaria->GetMaxFever();
        patient->fever.push_back( max_fever > 0 ? max_fever + 37.0f : -1.0f );
        // GetInfectiousness is an inline function of IndividualHuman (not belonging to any queriable interface presently)
        patient->infectiousness.push_back( static_cast<IndividualHuman*>(context)->GetInfectiousness() );

        // True values in model
        patient->true_asexual_density.push_back( susceptibility_malaria->get_parasite_density() );
        patient->true_gametocyte_density.push_back( individual_malaria->GetGametocyteDensity() );

        // Values incorporating variability and sensitivity of blood tes
        patient->asexual_parasite_density.push_back( individual_malaria->CheckParasiteCountWithTest( MALARIA_TEST_BLOOD_SMEAR ) );
        patient->gametocyte_density.push_back( individual_malaria->CheckGametocyteCountWithTest( MALARIA_TEST_BLOOD_SMEAR ) );
        patient->rdt.push_back( individual_malaria->CheckParasiteCountWithTest( MALARIA_TEST_NEW_DIAGNOSTIC ) );

        // Positive fields of view (out of 200 views in Garki-like setup)
        int positive_asexual_fields = 0;
        int positive_gametocyte_fields = 0;
        individual_malaria->CountPositiveSlideFields( DLL_HELPER.RNG, 200, (float)(1.0/400.0), positive_asexual_fields, positive_gametocyte_fields);
        patient->pos_asexual_fields.push_back( (float)positive_asexual_fields );
        patient->pos_gametocyte_fields.push_back( (float)positive_gametocyte_fields );
        LOG_DEBUG_F("(a,g) = (%d,%d)\n", (int)positive_asexual_fields, (int)positive_gametocyte_fields);

        return true;
    }

    void MalariaSurveyJSONAnalyzer::WriteOutput( float currentTime )
    {
        // Open output file
        ofstream ofs;
        std::ostringstream output_file_name;
        output_file_name << GetBaseOutputFilename() << "_" << (m_report_count-1) << ".json";
        LOG_INFO_F( "Writing file: %s\n", output_file_name.str().c_str() );
        ofs.open( FileSystem::Concat( EnvPtr->OutputPath, output_file_name.str() ).c_str() );
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
        LOG_DEBUG("Beginning malaria patient array.\n");
        pIJsonObj->BeginObject();
        pIJsonObj->Insert("ntsteps", m_reporting_interval);
        pIJsonObj->Insert("patient_array");
        pIJsonObj->BeginArray();
        for(auto &id_patient_pair: m_patient_map)
        {
            MalariaPatient* patient = id_patient_pair.second;
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

    void MalariaSurveyJSONAnalyzer::ClearOutputData()
    {
        for( auto &patient: m_patient_map )
        {
            delete patient.second;
        }
        m_patient_map.clear();
    }
}