/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <string>
#include "ReportHIVART.h"
#include "Log.h"
#include "Exceptions.h"
#include "NodeEventContext.h"
#include "IndividualEventContext.h"
#include "Contexts.h"
#include "FileSystem.h"
#include "IIndividualHumanHIV.h"
#include "SusceptibilityHIV.h"
#include "IHIVInterventionsContainer.h"
#include "SimulationEnums.h"

static const char* _module = "ReportHIVART";

namespace Kernel
{
    IReport* ReportHIVART::Create( ISimulation* sim )
    {
        return new ReportHIVART( sim );
    }

    ReportHIVART::ReportHIVART( ISimulation* sim )
        : BaseTextReportEvents("ReportHIVART.csv")
        , simulation( sim )
    {
        // default constructor listens to nothing
        eventTriggerList.push_back( "StartedART" );
        eventTriggerList.push_back( "StoppedART" );
    }

    ReportHIVART::~ReportHIVART()
    {
    }

    std::string ReportHIVART::GetHeader() const 
    {
        std::stringstream header ;
        header << "Year"        << ","
               << "Node_ID"     << ","
               << "ID"          << ","
               << "Age"         << ","
               << "Gender"      << ","
               << "CD4"         << ","
               << "StartingART";

        return header.str();
    }

    bool ReportHIVART::notifyOnEvent(IIndividualHumanEventContext *context, const std::string& StateChange)
    {
        /*IIndividualHumanHIV * iindividual_hiv = nullptr;
        if (s_OK != context->QueryInterface(GET_IID(IIndividualHumanHIV), (void**)&iindividual_hiv) )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "context", "IIndividualHumanHIV", "IIndividualHumanEventContext");
        }*/

        IHIVMedicalHistory * med_parent = nullptr;
        if (context->GetInterventionsContext()->QueryInterface(GET_IID(IHIVMedicalHistory), (void**)&med_parent) != s_OK)
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IHIVMedicalHistory", "IIndividualHumanContext" );
        }

        float            sim_year = simulation->GetSimulationTime().Year();
        ExternalNodeId_t node_id  = context->GetNodeEventContext()->GetNodeContext()->GetExternalID();
        int              id       = context->GetSuid().data;
        float            age      = context->GetAge();
        bool             gender   = (context->GetGender() == Gender::MALE) ? 0 : 1 ;
        //float            cd4count = iindividual_hiv->GetHIVSusceptibility()->GetCD4count();
        float            cd4count = med_parent->LastRecordedCD4();

        bool startingART = 1;
        if( StateChange == "StoppedART" )
        {
            startingART = 0;
        }

        GetOutputStream() << sim_year    << "," 
                          << node_id     << ","
                          << id          << ","
                          << age         << ","
                          << gender      << ","
                          << cd4count    << ","
                          << startingART << std::endl ;

        return true ;
    }
}
