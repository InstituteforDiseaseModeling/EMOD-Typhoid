/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "PolioNodeSurvey.h"

#include "EventCoordinator.h"
#include "NodeEventContext.h"
#include "Log.h"
#include "Configure.h"
#include "IndividualPolio.h"
#include "Sugar.h"
#include "FileSystem.h"

static const char* _module = "PolioNodeSurvey";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(PolioNodeSurvey)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(PolioNodeSurvey)

    IMPLEMENT_FACTORY_REGISTERED(PolioNodeSurvey)

    PolioNodeSurvey::PolioNodeSurvey()
        : m_parent(nullptr)
        , m_outputFilename("surveydata.bin")
        , m_surveyResults(nullptr)
    {
        initSimTypes( 1, "POLIO_SIM" );
    }

    PolioNodeSurvey::~PolioNodeSurvey()
    {
        // This function is intentionally left blank.
    }

    int PolioNodeSurvey::AddRef() { return BaseNodeIntervention::AddRef(); }
    int PolioNodeSurvey::Release() { return BaseNodeIntervention::Release(); }

    bool
        PolioNodeSurvey::Configure(
        const Configuration * inputJson
        )
    {
/*
        initConfigTypeMap("Output", &m_outputFilename, PNS_Output_DESC_TEXT );
*/
        return JsonConfigurable::Configure( inputJson );
    }

    bool PolioNodeSurvey::Distribute(INodeEventContext *context, IEventCoordinator2* pEC)
    {
        LOG_INFO("PolioNodeSurvey distributed.\n");

        GetInvariantStats(context);
        GetIndividualStats(context, pEC);
        LogSurveyCompletion();
        RecordSurveyData();
        ReleaseSurveyDataMemory();
        ConditionallyWriteSurveyDataSchema();

        // It is important to return false here so the intervention is ephemeral and the memory for
        // the survey data will be freed.
        return true;
    }

    void PolioNodeSurvey::SetContextTo(INodeEventContext *context)
    {
        m_parent = context;
    }

    void PolioNodeSurvey::Update(float dt)
    {
        // This function is intentionally left blank.
    }

    // This data doesn't vary by individual, so we acquire once.
    void PolioNodeSurvey::GetInvariantStats( INodeEventContext * context )
    {
        m_currentTime = context->GetTime().time;
        m_nodeId      = context->GetExternalId();
    }

    void PolioNodeSurvey::GetIndividualStats( INodeEventContext * context, IEventCoordinator2* pEC )
    {
        float coverage = pEC->GetDemographicCoverage();
        ::RANDOMBASE *prng = context->GetRng();

        INodeEventContext::individual_visit_function_t visit_func = [this, pEC, coverage, prng](IIndividualHumanEventContext *ihec)
        {
            bool qualifies = IndividualInDemographic(ihec, pEC);

            if (qualifies && (coverage > prng->e()))
            {
                SurveyResults *pSurveyResults    = this->m_surveyResults + this->m_humanIndex;
                pSurveyResults->time             = this->m_currentTime;
                pSurveyResults->nodeId           = this->m_nodeId;
                pSurveyResults->individualId     = ihec->GetSuid().data;
                pSurveyResults->individualAge    = ihec->GetAge();
                pSurveyResults->monteCarloWeight = (float)ihec->GetMonteCarloWeight();

                IIndividualHumanInterventionsContext *iiic = ihec->GetInterventionsContext();
                IIndividualHumanContext *iihc = iiic->GetParent();
                IIndividualHumanPolio *iihp;
                iihc->QueryInterface(GET_IID(IIndividualHumanPolio), (void**)&iihp);

                auto ispr = iihp->GetSusceptibilityReporting();

                ispr->GetSheddingTiter(pSurveyResults->sheddingTiter);
                ispr->GetHumoralNAb(pSurveyResults->humoralNAb);
                ispr->GetMucosalNAb(pSurveyResults->mucosalNAb);
                ispr->GetMaternalSerumNAb(pSurveyResults->maternalSerumNAb);
                ispr->GetHumoralMemoryNAb(pSurveyResults->humoralMemoryNAb);
                ispr->GetMucosalMemoryNAb(pSurveyResults->mucosalMemoryNAb);
                ispr->GetTimeSinceLastInfection(pSurveyResults->timeSinceLastInfection);
                ispr->GetVaccineDosesReceived(pSurveyResults->vaccineDosesReceived);
                pSurveyResults->individualAcquireRisk = ispr->GetIndividualAcquireRisk();
                memcpy(pSurveyResults->infectionStrains, ispr->GetInfectionStrains(), sizeof(int)*N_POLIO_VIRUS_TYPES);
                memcpy(pSurveyResults->newInfectionsByStrain, ispr->GetNewInfectionsByStrain(), sizeof(int)*N_POLIO_VIRUS_TYPES);

                this->m_humanIndex++;
            }
        };

        int currentNodePopulationCount = context->GetIndividualHumanCount();
        m_surveyResults = _new_ SurveyResults[currentNodePopulationCount];

        m_humanIndex = 0;
        context->VisitIndividuals(visit_func);
    }

    void PolioNodeSurvey::RecordSurveyData()
    {
        // --Transmit the node information to rank 0 for reporting--
        // Write locally since we don't yet have a mechanism for aggregating and reducing on rank 0
        char filename[256];
#ifdef WIN32
        sprintf_s(filename, sizeof(filename), "%s\\survey%d-%d.bin", EnvPtr->OutputPath.c_str(), m_nodeId, (int)m_currentTime);
#else
        sprintf(filename, "%s\\survey%d-%d.bin", EnvPtr->OutputPath.c_str(), m_nodeId, (int)m_currentTime);
#endif

        std::ofstream surveyData;
        surveyData.open(filename, ios_base::out | ios_base::trunc | ios_base::binary);

        if (surveyData)
        {
            int retries = 0;
            // write the survey contents to file
            while ( !surveyData.write( (const char *)m_surveyResults, m_humanIndex * sizeof(SurveyResults) ) || !surveyData.good() )
            {
                // retry if error max 10x
                LOG_WARN_F("Fail to write to PolioNodeSurvey file. Time:%f Node:%d", m_currentTime, m_nodeId);
                surveyData.clear();

                if(retries++ > 10)
                {
                    LOG_ERR_F("PolioNodeSurvey  surveyData.write() failed [good = %d, bad = %d, fail = %d, eof = %d].\n", surveyData.good(), surveyData.bad(), surveyData.fail(), surveyData.eof());
                    break;
                }
            }
            surveyData.close();
        }
    }

    void PolioNodeSurvey::LogSurveyCompletion()
    {
        char buffer[256];
#ifdef WIN32
        sprintf_s(buffer, sizeof(buffer), "Visited %d individuals in node %d at time %f.\n", m_humanIndex, m_nodeId, m_currentTime);
#else
        sprintf(buffer, "Visited %d individuals in node %d at time %f.\n", m_humanIndex, m_nodeId, m_currentTime);
#endif
        LOG_INFO(buffer);
    }

    void PolioNodeSurvey::ConditionallyWriteSurveyDataSchema()
    {
        if (EnvPtr->MPI.Rank == 0)
        {
            std::ofstream schema;
            std::string filename = FileSystem::Concat( EnvPtr->OutputPath, std::string("surveyschema.json") );

            // Try opening the schema file for reading. Only if we're _not_ successful do we
            // write the schema to disk.

            schema.open(filename, ios_base::in);
            if (!schema.is_open())
            {
                schema.open(filename, ios_base::out | ios_base::trunc);
                schema.write(SurveyResults::GetSchema(), strlen(SurveyResults::GetSchema()));
                schema.close();
            }
            else
            {
                schema.close();
            }
        }
    }

    void PolioNodeSurvey::ReleaseSurveyDataMemory()
    {
        delete [] m_surveyResults;
        m_surveyResults = nullptr;
    }

    bool PolioNodeSurvey::IndividualInDemographic( IIndividualHumanEventContext *ihec, IEventCoordinator2* pEC )
    {
        bool qualifies = true;

        switch (pEC->GetTargetDemographic())
        {
        case TargetDemographicType::PossibleMothers:
            if (!ihec->IsPossibleMother())
            {
                LOG_DEBUG("Individual not surveyed because of demographic (not possible mother).\n");
                qualifies = false;
            }
            break;

        case TargetDemographicType::ExplicitAgeRanges:
            if (ihec->GetAge() < (pEC->GetMinimumAge() * DAYSPERYEAR))
            {
                LOG_DEBUG_F("Individual not surveyed because of demographic (too young, %f < %f).\n", ihec->GetAge(), pEC->GetMinimumAge());
                qualifies = false;
            }
            else if (ihec->GetAge() > (pEC->GetMaximumAge() * DAYSPERYEAR))
            {
                LOG_DEBUG_F("Individual not surveyed because of demographic (too old, %f > %f).\n", ihec->GetAge(), pEC->GetMaximumAge());
                qualifies = false;
            }
            break;

        default:
            break;
        }

        return qualifies;
    }

    char *_surveyResultsSchema =
        "{"
            "\"time\"                   :{\"type\":\"float\",\"size\":4,\"count\":1,\"offset\":0},"
            "\"nodeId\"                 :{\"type\":\"int\",  \"size\":4,\"count\":1,\"offset\":4},"
            "\"individualId\"           :{\"type\":\"int\",  \"size\":4,\"count\":1,\"offset\":8},"
            "\"individualAge\"          :{\"type\":\"float\",\"size\":4,\"count\":1,\"offset\":12},"
            "\"monteCarloWeight\"       :{\"type\":\"float\",\"size\":4,\"count\":1,\"offset\":16},"
            "\"sheddingTiter\"          :{\"type\":\"float\",\"size\":4,\"count\":6,\"offset\":20},"
            "\"humoralNAb\"             :{\"type\":\"float\",\"size\":4,\"count\":3,\"offset\":44},"
            "\"mucosalNAb\"             :{\"type\":\"float\",\"size\":4,\"count\":3,\"offset\":56},"
            "\"maternalSerumNAb\"       :{\"type\":\"float\",\"size\":4,\"count\":3,\"offset\":68},"
            "\"humoralMemoryNAb\"       :{\"type\":\"float\",\"size\":4,\"count\":3,\"offset\":80},"
            "\"mucosalMemoryNAb\"       :{\"type\":\"float\",\"size\":4,\"count\":3,\"offset\":92},"
            "\"timeSinceLastInfection\" :{\"type\":\"float\",\"size\":4,\"count\":3,\"offset\":104},"
            "\"vaccineDosesReceived\"   :{\"type\":\"int\",  \"size\":4,\"count\":6,\"offset\":116},"
            "\"infectionStrains\"       :{\"type\":\"int\",  \"size\":4,\"count\":6,\"offset\":140},"
            "\"newInfectionsByStrain\"  :{\"type\":\"int\",  \"size\":4,\"count\":6,\"offset\":164},"
            "\"individualAcquireRisk\"  :{\"type\":\"float\",\"size\":4,\"count\":1,\"offset\":188}"
        "}";

    const char *SurveyResults::GetSchema()
    {
        return _surveyResultsSchema;
    }

}
