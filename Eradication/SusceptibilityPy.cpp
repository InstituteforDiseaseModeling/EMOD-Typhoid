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


#include "SusceptibilityPy.h"

static const char * _module = "SusceptibilityPy";

#ifdef ENABLE_PYTHON

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Py.Susceptibility,SusceptibilityPyConfig)
    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPyConfig)
    END_QUERY_INTERFACE_BODY(SusceptibilityPyConfig)

    bool
    SusceptibilityPyConfig::Configure(
        const Configuration* config
    )
    {
        LOG_DEBUG( "Configure\n" );
        //initConfigTypeMap( "x_Population_Immunity",             &x_population_immunity, x_Population_Immunity_DESC_TEXT, 0.0f, 1.0f, 1.0f );
        return JsonConfigurable::Configure( config );
    }

    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPy)
        HANDLE_INTERFACE(ISusceptibilityPy)
        HANDLE_INTERFACE(ISusceptibilityPyReportable)
    END_QUERY_INTERFACE_BODY(SusceptibilityPy)

    SusceptibilityPy::SusceptibilityPy(IIndividualHumanContext *context)
        : Susceptibility(context) 
    {
        // Everything initialized to 0 in Initialize
    }

    void SusceptibilityPy::Initialize(float _age, float _immmod, float _riskmod)
    {
        LOG_DEBUG_F( "Initializing Py immunity object for new individual: id=%lu, age=%f, immunity modifier=%f, risk modifier=%f\n", parent->GetSuid().data, _age, _immmod, _riskmod );
        Susceptibility::Initialize(_age, _immmod, _riskmod);

        // throws exception on error, no return type. 
    }

    SusceptibilityPy::~SusceptibilityPy(void)
    {
    }

    SusceptibilityPy *SusceptibilityPy::CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod)
    {
        //LOG_DEBUG_F( "Creating Py immunity object for new individual: age=%f, immunity modifier=%f, risk modifier=%f\n", age, immmod, riskmod );
        SusceptibilityPy *newsusceptibility = _new_ SusceptibilityPy(context);
        newsusceptibility->Initialize(age, immmod, riskmod);

        return newsusceptibility;
    }

    void SusceptibilityPy::Update(float dt)
    {
        age += dt; // tracks age for immune purposes
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::SusceptibilityPy)

namespace Kernel {
/*
    template<class Archive>
    void serialize(Archive & ar, SusceptibilityPy &sus, const unsigned int  file_version )
    {
        ar & boost::serialization::base_object<SusceptibilityEnvironmental>(sus);
}
*/
}
#endif

#endif // ENABLE_PYTHON