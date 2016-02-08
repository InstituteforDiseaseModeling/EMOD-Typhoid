/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#include "stdafx.h"
#include <string>


/*#include "Debug.h"
#include "MathFunctions.h"
#include "Environment.h"
#include "Types.h"

#include "Interventions.h"
#include "NodeDemographics.h"
#include "Common.h"
#include "Exceptions.h"
#include "Individual.h" // for IIndividualHumanEventContext
#include "NodeDemographics.h" // for static strings e.g. tOPV_dose_distribution
#include "RANDOM.h"
#include "SimulationConfig.h"
#include "Vaccine.h"
*/

#include "SusceptibilityPyDemo.h"

static const char * _module = "SusceptibilityPyDemo";
#ifdef ENABLE_POLIO

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(PyDemo.Susceptibility,SusceptibilityPyDemoConfig)
    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPyDemoConfig)
    END_QUERY_INTERFACE_BODY(SusceptibilityPyDemoConfig)

    bool
    SusceptibilityPyDemoConfig::Configure(
        const Configuration* config
    )
    {
        LOG_DEBUG( "Configure\n" );
        //initConfigTypeMap( "x_Population_Immunity",             &x_population_immunity, x_Population_Immunity_DESC_TEXT, 0.0f, 1.0f, 1.0f );
        return JsonConfigurable::Configure( config );
    }

    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPyDemo)
        HANDLE_INTERFACE(ISusceptibilityPyDemo)
        HANDLE_INTERFACE(ISusceptibilityPyDemoReportable)
    END_QUERY_INTERFACE_BODY(SusceptibilityPyDemo)

    /*SusceptibilityPyDemo::SusceptibilityPyDemo()
        : SusceptibilityEnvironmental()
    {
    }*/

    SusceptibilityPyDemo::SusceptibilityPyDemo(IIndividualHumanContext *context)
        : SusceptibilityEnvironmental(context) 
    {
        // Everything initialized to 0 in Initialize
    }

    void SusceptibilityPyDemo::Initialize(float _age, float _immmod, float _riskmod)
    {
        LOG_DEBUG_F( "Initializing PyDemo immunity object for new individual: id=%lu, age=%f, immunity modifier=%f, risk modifier=%f\n", parent->GetSuid().data, _age, _immmod, _riskmod );
        SusceptibilityEnvironmental::Initialize(_age, _immmod, _riskmod);

        // throws exception on error, no return type. 
    }

    SusceptibilityPyDemo::~SusceptibilityPyDemo(void)
    {
    }

    SusceptibilityPyDemo *SusceptibilityPyDemo::CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod)
    {
        //LOG_DEBUG_F( "Creating PyDemo immunity object for new individual: age=%f, immunity modifier=%f, risk modifier=%f\n", age, immmod, riskmod );
        SusceptibilityPyDemo *newsusceptibility = _new_ SusceptibilityPyDemo(context);
        newsusceptibility->Initialize(age, immmod, riskmod);

        return newsusceptibility;
    }

    void SusceptibilityPyDemo::Update(float dt)
    {
        age += dt; // tracks age for immune purposes
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::SusceptibilityPyDemo)
/*
namespace Kernel {

    template<class Archive>
    void serialize(Archive & ar, SusceptibilityPyDemo &sus, const unsigned int  file_version )
    {
        ar & boost::serialization::base_object<SusceptibilityEnvironmental>(sus);
    }
}*/
#endif

#endif // ENABLE_POLIO
