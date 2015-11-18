/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "InterventionFactory.h"
#include "Interventions.h"
#include "PolioDefs.h"
#include "suids.hpp"

namespace Kernel
{
    class SurveyResults
    {
    public:
        SurveyResults() {};
        static const char *GetSchema();

        // ** Don't add or remove anything from here without updating the output of GetSchema(). **
        float   time;
        int32_t nodeId;
        int32_t individualId;
        float   individualAge;
        float   monteCarloWeight;
        float   sheddingTiter[6];
        float   humoralNAb[3];
        float   mucosalNAb[3];
        float   maternalSerumNAb[3];
        float   humoralMemoryNAb[3];
        float   mucosalMemoryNAb[3];
        float   timeSinceLastInfection[3];
        int32_t vaccineDosesReceived[6];
        int32_t infectionStrains[ N_POLIO_VIRUS_TYPES ];
        int32_t newInfectionsByStrain[ N_POLIO_VIRUS_TYPES ];
        float   individualAcquireRisk;
        // ** Don't add or remove anything from here without updating the output of GetSchema(). **
    };

    class PolioNodeSurvey
        : public BaseNodeIntervention
    {
    protected:
        PolioNodeSurvey();

    public:
        virtual ~PolioNodeSurvey();
        virtual int AddRef();
        virtual int Release();
        bool Configure( const Configuration* config );

        // InterventionFactory
        DECLARE_FACTORY_REGISTERED(InterventionFactory, PolioNodeSurvey, INodeDistributableIntervention)

/*
        // JsonConfigurable
        DECLARE_CONFIGURED(PolioNodeSurvey)
*/

        // INodeDistributableIntervention
        virtual bool Distribute(INodeEventContext *context, IEventCoordinator2* pEC /* = nullptr */);

        void ReleaseSurveyDataMemory();

       virtual void SetContextTo(INodeEventContext *context);
        virtual void Update(float dt);

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **pinstance);
//        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        void GetInvariantStats( INodeEventContext * context );
        void GetIndividualStats( INodeEventContext * context, IEventCoordinator2* pEC );

        static bool IndividualInDemographic( IIndividualHumanEventContext *ihec, IEventCoordinator2* pEC );

        void LogSurveyCompletion();
        void RecordSurveyData();
        void ConditionallyWriteSurveyDataSchema();

    protected:
        INodeEventContext *m_parent;
        string m_outputFilename;

        float m_currentTime;
        uint32_t m_nodeId;
        int m_humanIndex;
        SurveyResults *m_surveyResults;
    };
}
