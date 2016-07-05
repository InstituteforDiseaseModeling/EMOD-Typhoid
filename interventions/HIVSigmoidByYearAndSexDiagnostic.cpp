/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "HIVSigmoidByYearAndSexDiagnostic.h"

#include "NodeEventContext.h"  // for INodeEventContext
#include "SimulationEnums.h"
#include "Sigmoid.h"

static const char * _module = "HIVSigmoidByYearAndSexDiagnostic";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(HIVSigmoidByYearAndSexDiagnostic)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        //HANDLE_INTERFACE(IHealthSeekingBehavior)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(HIVSigmoidByYearAndSexDiagnostic)

    IMPLEMENT_FACTORY_REGISTERED(HIVSigmoidByYearAndSexDiagnostic)

    HIVSigmoidByYearAndSexDiagnostic::HIVSigmoidByYearAndSexDiagnostic()
    : rampMin(0)     // initialized below by initConfigTypeMap
    , rampMax(1)
    , rampMidYear(2000)
    , rampRate(1)
    , femaleMultiplier(1)
    {
        initConfigTypeMap("Ramp_Min", &rampMin, HIV_Ramp_Min_DESC_TEXT , -1, 1, 0);
        initConfigTypeMap("Ramp_Max", &rampMax, HIV_Ramp_Max_DESC_TEXT , -1, 1, 1);
        initConfigTypeMap("Ramp_MidYear", &rampMidYear, HIV_Ramp_MidYear_DESC_TEXT , MIN_YEAR, MAX_YEAR, 2000);
        initConfigTypeMap("Ramp_Rate", &rampRate, HIV_Ramp_Rate_DESC_TEXT , -100, 100, 1);
        initConfigTypeMap("Female_Multiplier", &femaleMultiplier, HIV_Female_Multiplier_DESC_TEXT , 0, FLT_MAX, 1);
    }

    HIVSigmoidByYearAndSexDiagnostic::HIVSigmoidByYearAndSexDiagnostic( const HIVSigmoidByYearAndSexDiagnostic& master )
        : HIVSimpleDiagnostic( master )
    {
        rampMin = master.rampMin;
        rampMax = master.rampMax;
        rampMidYear = master.rampMidYear;
        rampRate = master.rampRate;
        femaleMultiplier = master.femaleMultiplier;
    }

    bool
    HIVSigmoidByYearAndSexDiagnostic::positiveTestResult()
    {
        //LOG_DEBUG_F("About to issue positiveTestResult\n");

        float year = parent->GetEventContext()->GetNodeEventContext()->GetTime().Year();
        auto gender = parent->GetEventContext()->GetGender();

        float valueMultiplier = (gender == Gender::FEMALE) ? femaleMultiplier : 1;
        float value_tmp = Sigmoid::variableWidthAndHeightSigmoid( year, rampMidYear, rampRate, rampMin, rampMax );
        float value = max( 0.0f, value_tmp ) * valueMultiplier;

        LOG_DEBUG_F("min=%f, max=%f, rate=%f, midYear=%f, multiplier=%f, year=%f\n", rampMin, rampMax, rampRate, rampMidYear, valueMultiplier, year);
        LOG_DEBUG_F("rampMin + (rampMax-rampMin)/(1.0 + exp(-rampRate*(year-rampMidYear))) = %f\n", value_tmp );

        bool testResult = ( SMART_DRAW( value) );
        LOG_DEBUG_F("Individual %d: sex=%d, year=%f, value=%f, returning %d.\n", parent->GetSuid().data, gender, year, value, testResult);
 
        return testResult;
    }

    REGISTER_SERIALIZABLE(HIVSigmoidByYearAndSexDiagnostic);

    void HIVSigmoidByYearAndSexDiagnostic::serialize(IArchive& ar, HIVSigmoidByYearAndSexDiagnostic* obj)
    {
        HIVSimpleDiagnostic::serialize( ar, obj );
        HIVSigmoidByYearAndSexDiagnostic& cascade = *obj;

        ar.labelElement("rampMin"         ) & cascade.rampMin;
        ar.labelElement("rampMax"         ) & cascade.rampMax;
        ar.labelElement("rampMidYear"     ) & cascade.rampMidYear;
        ar.labelElement("rampRate"        ) & cascade.rampRate;
        ar.labelElement("femaleMultiplier") & cascade.femaleMultiplier;
    }
}
