/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>
#include <map>

#include "BaseEventReportIntervalOutput.h"

namespace Kernel
{
    struct MalariaPatient
    {
        MalariaPatient(int id_, float age_, float local_birthday_);
        ~MalariaPatient();

        int id;
        float initial_age;
        float local_birthday;
        std::vector< std::pair<int,int> > strain_ids;
        std::vector<float> true_asexual_density;
        std::vector<float> true_gametocyte_density;
        std::vector<float> asexual_parasite_density;
        std::vector<float> gametocyte_density;
        std::vector<float> infectiousness;
        std::vector<float> pos_asexual_fields;
        std::vector<float> pos_gametocyte_fields;
        std::vector<float> fever;
        std::vector<float> rdt;

        // IJsonSerializable
        virtual void JSerialize( IJsonObjectAdapter* root, JSerializer* helper );

    protected:
        void SerializeChannel( std::string channel_name, std::vector<float> &channel_data,
                               IJsonObjectAdapter* root, JSerializer* helper );

    };

    class MalariaSurveyJSONAnalyzer : public BaseEventReportIntervalOutput
    {
    public:
        MalariaSurveyJSONAnalyzer();
        virtual ~MalariaSurveyJSONAnalyzer();

        // BaseEventReportIntervalOutput
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, const std::string& StateChange );

    protected:
        // BaseEventReportIntervalOutput
        virtual void WriteOutput( float currentTime );
        virtual void ClearOutputData();

        typedef std::map<int, MalariaPatient*> patient_map_t;
        patient_map_t m_patient_map;
    };
}