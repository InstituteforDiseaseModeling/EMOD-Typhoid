/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BaseTextReportEvents.h"
#include "SimulationEnums.h"

namespace Kernel
{
    ENUM_DEFINE(CD4_Stage,
        ENUM_VALUE_SPEC(HIV_NEGATIVE   , 0) 
        ENUM_VALUE_SPEC(CD4_UNDER_200  , 1) 
        ENUM_VALUE_SPEC(CD4_200_TO_350 , 2) 
        ENUM_VALUE_SPEC(CD4_350_TO_500 , 3)
        ENUM_VALUE_SPEC(CD4_ABOVE_500  , 4)
        ENUM_VALUE_SPEC(COUNT          , 5) )   // Needed for array initialization below

    ENUM_DEFINE(Care_Stage,
        ENUM_VALUE_SPEC(NA                                  , 0) 
        ENUM_VALUE_SPEC(NOT_DIAGNOSED                       , 1) 
        ENUM_VALUE_SPEC(DIAGNOSED_BUT_NEVER_LINKED          , 2) 
        ENUM_VALUE_SPEC(DIAGNOSED_AND_LINKED_BUT_NOT_IN_CARE, 3) 
        ENUM_VALUE_SPEC(IN_CARE_BUT_NEVER_INITIATED_ART     , 4)
        ENUM_VALUE_SPEC(ON_ART_SIX_OR_FEWER_MONTHS          , 5)
        ENUM_VALUE_SPEC(ON_ART_MORE_THAN_SIX_MONTHS         , 6)
        ENUM_VALUE_SPEC(INITIATED_ART_BUT_NO_LONGER_ON_ART  , 7)
        ENUM_VALUE_SPEC(COUNT                               , 8) )


    class Report_HIV_WHO2015 : public BaseTextReportEvents
    {
    public:
        Report_HIV_WHO2015();
        virtual ~Report_HIV_WHO2015();

        // BaseEventReport
        virtual bool Configure( const Configuration* ) override;
        //virtual void BeginTimestep() ;

        virtual void UpdateEventRegistration( float currentTime, 
                                              float dt, 
                                              std::vector<INodeEventContext*>& rNodeEventContextList ) override;

        virtual std::string GetHeader() const override;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override;
        virtual void LogIndividualData( Kernel::IIndividualHuman* individual ) override;
        virtual void LogNodeData( Kernel::INodeContext * pNC ) override;
        //virtual void Reduce();
        //virtual void Finalize();
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, 
                                    const std::string& StateChange ) override;
    private:

        const float report_hiv_half_period;
        float next_report_time;
        bool doReport;
        float startYear;
        float stopYear;
        bool is_collecting_data;

        CD4_Stage::Enum ComputeCD4Stage(IIndividualHumanEventContext *context);
        Care_Stage::Enum ComputeCareStage(IIndividualHumanEventContext *context);

        float Population[CD4_Stage::Enum::COUNT][Care_Stage::Enum::COUNT];      // CD4_Stage, Care_Stage --> Population
        float DiseaseDeaths[CD4_Stage::Enum::COUNT][Care_Stage::Enum::COUNT];   // CD4_Stage, Care_Stage --> DiseaseDeaths
        float NonDiseaseDeaths[CD4_Stage::Enum::COUNT][Care_Stage::Enum::COUNT];// CD4_Stage, Care_Stage --> NonDiseaseDeaths
        float ART_Initiations[CD4_Stage::Enum::COUNT];                          // CD4_Stage             --> ART initiations
        float New_Infections;                                                   //                       --> Infections
        float New_Diagnoses[CD4_Stage::Enum::COUNT];                            //                       --> Diagnoses
        float PreART_Dropouts[CD4_Stage::Enum::COUNT];                          //                       --> Dropouts from Pre-ART
        float ART_Dropouts;                                                     //                       --> Dropouts from ART
    };
}
