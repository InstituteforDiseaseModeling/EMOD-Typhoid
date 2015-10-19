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
    typedef std::string serotype_name_t;
    typedef std::vector<float> channel_data_t;
    typedef std::map<serotype_name_t, channel_data_t> serotype_channel_data_t;

    struct PolioPatient
    {
        PolioPatient(int id_, float age_);
        ~PolioPatient();

        int id;
        float initial_age;

        // map by serotype (PV1, PV2, PV3)
        serotype_channel_data_t mucosal_Nab;     // (reciprocal titer, linear units)
        serotype_channel_data_t humoral_Nab;     // (reciprocal titer, linear units)
        serotype_channel_data_t mucosalmemory_Nab;
        serotype_channel_data_t humoralmemory_Nab;
        serotype_channel_data_t maternalserumNAb;

        // map by virus type (WPV1, WPV2, WPV3, VRPV1, VRPV2, VRPV3)
        serotype_channel_data_t shedding_titer;  // (TCID50/g feces) amount of virus currently being shed

        // IJsonSerializable
        virtual void JSerialize( IJsonObjectAdapter* root, JSerializer* helper );

    protected:
        void SerializeChannel( std::string channel_name, 
                               serotype_channel_data_t &channel_data,
                               IJsonObjectAdapter* root, 
                               JSerializer* helper );
    };

    class PolioSurveyJSONAnalyzer : public BaseEventReportIntervalOutput
    {
    public:
        PolioSurveyJSONAnalyzer();
        virtual ~PolioSurveyJSONAnalyzer();

        // BaseEventReportIntervalOutput
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, const std::string& StateChange );

    protected:
        // BaseEventReportIntervalOutput
        virtual void WriteOutput( float currentTime );
        virtual void ClearOutputData();

    private:

        typedef std::map<int, PolioPatient*> patient_map_t;
        patient_map_t m_patient_map;

        long m_node_id;
        std::map<int,std::string> m_pst_map; 
        std::map<int,std::string> m_pvt_map; 
    };
}
