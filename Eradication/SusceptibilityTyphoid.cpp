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

#include "SusceptibilityTyphoid.h"
#include "SimulationConfig.h"

static const char * _module = "SusceptibilityTyphoid";
#ifdef ENABLE_TYPHOID

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Typhoid.Susceptibility,SusceptibilityTyphoidConfig)
    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityTyphoidConfig)
    END_QUERY_INTERFACE_BODY(SusceptibilityTyphoidConfig)

    bool
    SusceptibilityTyphoidConfig::Configure(
        const Configuration* config
    )
    {
        LOG_DEBUG( "Configure\n" );
        //initConfigTypeMap( "x_Population_Immunity",             &x_population_immunity, x_Population_Immunity_DESC_TEXT, 0.0f, 1.0f, 1.0f );
        return JsonConfigurable::Configure( config );
    }

    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityTyphoid)
        HANDLE_INTERFACE(ISusceptibilityTyphoid)
        HANDLE_INTERFACE(ISusceptibilityTyphoidReportable)
    END_QUERY_INTERFACE_BODY(SusceptibilityTyphoid)

    /*SusceptibilityTyphoid::SusceptibilityTyphoid()
        : SusceptibilityEnvironmental()
    {
    }*/

    SusceptibilityTyphoid::SusceptibilityTyphoid(IIndividualHumanContext *context)
        : SusceptibilityEnvironmental(context) 
    {
        // Everything initialized to 0 in Initialize
    }

    void SusceptibilityTyphoid::Initialize(float _age, float _immmod, float _riskmod)
    {
        LOG_DEBUG_F( "Initializing Typhoid immunity object for new individual: id=%lu, age=%f, immunity modifier=%f, risk modifier=%f\n", parent->GetSuid().data, _age, _immmod, _riskmod );
        SusceptibilityEnvironmental::Initialize(_age, _immmod, _riskmod);

        
       if( _age == 0.0f )
        {
            mod_acquire = 0.0f;
            LOG_DEBUG_F( "Newborn being made immune for now.\n" );
        
        // throws exception on error, no return type. 
        }
    }

    SusceptibilityTyphoid::~SusceptibilityTyphoid(void)
    {
    }

    SusceptibilityTyphoid *SusceptibilityTyphoid::CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod)
    {
        //LOG_INFO_F( "Creating Typhoid immunity object for new individual: age=%f, immunity modifier=%f, risk modifier=%f\n", age, immmod, riskmod );
        SusceptibilityTyphoid *newsusceptibility = _new_ SusceptibilityTyphoid(context);
        newsusceptibility->Initialize(age, immmod, riskmod);

        return newsusceptibility;
    }

    void SusceptibilityTyphoid::Update(float dt)
    {
        age += dt; // tracks age for immune purposes
         // if cross N year age boundary

        //end of maternal antibodies at 6 months
        float age_boundary =  0.5f * DAYSPERYEAR;
        if( age >= age_boundary && age-dt< age_boundary && mod_acquire == 0 )
        {
           if( randgen->e() < GET_CONFIGURABLE(SimulationConfig)->typhoid_6month_susceptible_fraction)
            {
                LOG_DEBUG_F( "1-yo being made susceptible.\n" );
                mod_acquire = 1.0f;
            }
        }

        float age_boundary_2 = 3 * DAYSPERYEAR;
        if( age >= age_boundary_2 && age-dt< age_boundary_2 && mod_acquire == 0 )
        {
           if( randgen->e() < GET_CONFIGURABLE(SimulationConfig)->typhoid_3year_susceptible_fraction )
            {
                LOG_DEBUG_F( "3-yo being made susceptible.\n" );
                mod_acquire = 1.0f;
            }
        }
		float age_boundary_3 = 6 * DAYSPERYEAR;
        if( age >= age_boundary_3 && age-dt< age_boundary_3 && mod_acquire == 0 )
        {
           if( randgen->e() < GET_CONFIGURABLE(SimulationConfig)->typhoid_6year_susceptible_fraction )
            {
                LOG_DEBUG_F( "6-yo being made susceptible.\n" );
                mod_acquire = 1.0f;
            }
        }

        float age_boundary_4 = 10 * DAYSPERYEAR;
        if( age >= age_boundary_4 && age-dt< age_boundary_4 && mod_acquire == 0 )
        {
                LOG_DEBUG_F( "Schoolkids being made susceptible.\n" );
                mod_acquire = 1.0f;
            }

        if( age > age_boundary_4+dt && mod_acquire == 0)
        {
            LOG_INFO_F("SOMEONE WAS MISSED AGE %f CUTOFF DAY %f MODACQUIRE %f\n", age, age_boundary_4+dt, mod_acquire);
        } 
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::SusceptibilityTyphoid)
/*
namespace Kernel {

    template<class Archive>
    void serialize(Archive & ar, SusceptibilityTyphoid &sus, const unsigned int  file_version )
    {
        ar & boost::serialization::base_object<SusceptibilityEnvironmental>(sus);
    }
}*/
#endif

#endif // ENABLE_TYPHOID
