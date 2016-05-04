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

#ifdef ENABLE_TYPHOID

#include "InfectionTyphoid.h"
#include "SusceptibilityTyphoid.h"
#include "InterventionsContainer.h"
#include "TyphoidDefs.h"
#include "Environment.h"
#include "Debug.h"

#include "Common.h"
#include "MathFunctions.h"
#include "RANDOM.h"
#include "Exceptions.h"
#include "SimulationConfig.h"
using namespace std;

static const char* _module = "InfectionTyphoid";

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Typhoid.Infection,InfectionTyphoidConfig)
    BEGIN_QUERY_INTERFACE_BODY(InfectionTyphoidConfig)
    END_QUERY_INTERFACE_BODY(InfectionTyphoidConfig)

    bool
    InfectionTyphoidConfig::Configure(
        const Configuration * config
    )
    {
        LOG_DEBUG( "Configure\n" );
        //initConfigTypeMap( "Enable_Contact_Tracing", &tracecontact_mode, Enable_Contact_Tracing_DESC_TEXT, false ); // polio
        bool bRet = JsonConfigurable::Configure( config );
        return bRet;
    }

    BEGIN_QUERY_INTERFACE_BODY(InfectionTyphoid)
        HANDLE_INTERFACE(IInfectionTyphoid)
    END_QUERY_INTERFACE_BODY(InfectionTyphoid)

    InfectionTyphoid::InfectionTyphoid()
    {
    }

    const SimulationConfig*
    InfectionTyphoid::params()
    {
        return GET_CONFIGURABLE(SimulationConfig);
    }

    InfectionTyphoid::InfectionTyphoid(IIndividualHumanContext *context) : InfectionEnvironmental(context)
    {
    }

    void InfectionTyphoid::Initialize(suids::suid _suid)
    {
        InfectionEnvironmental::Initialize(_suid);
    }

    InfectionTyphoid *InfectionTyphoid::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        //VALIDATE(boost::format(">InfTyphoid::CreateInfection(%1%, %2%)") % context->GetSuid().data % _suid.data );

        InfectionTyphoid *newinfection = _new_ InfectionTyphoid(context);
        newinfection->Initialize(_suid);

        return newinfection;
    }

    InfectionTyphoid::~InfectionTyphoid()
    {
    }

    void InfectionTyphoid::SetParameters(StrainIdentity* infstrain, int incubation_period_override)
    {
        InfectionEnvironmental::SetParameters(infstrain, incubation_period_override); // setup infection timers and infection state
        if(infstrain == NULL)
        {
            // using default strainIDs
            //infection_strain->SetAntigenID(default_antigen);
        }
        else
        {
            *infection_strain = *infstrain;
        }
    }

    void InfectionTyphoid::InitInfectionImmunology(Susceptibility* _immunity)
    {
        ISusceptibilityTyphoid* immunity = NULL;
        if( _immunity->QueryInterface( GET_IID( ISusceptibilityTyphoid ), (void**)&immunity ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "_immunity", "ISusceptibilityTyphoid", "Susceptibility" );
        }

        StateChange = InfectionStateChange::New;
        return InfectionEnvironmental::InitInfectionImmunology( _immunity );
    }

    void InfectionTyphoid::Update(float dt, Susceptibility* _immunity)
    {
        return;
        /*
        StateChange = InfectionStateChange::None;
        ISusceptibilityTyphoid* immunity = NULL;
        if( _immunity->QueryInterface( GET_IID( ISusceptibilityTyphoid ), (void**)&immunity ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "_immunity", "Susceptibility", "SusceptibilityTyphoid" );
        } */
        //return InfectionEnvironmental::Update( dt, _immunity );
    }

    void InfectionTyphoid::Clear()
    {
        StateChange = InfectionStateChange::Cleared;
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::InfectionTyphoid)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, InfectionTyphoid& inf, const unsigned int file_version )
    {
        ar & boost::serialization::base_object<Kernel::InfectionEnvironmental>(inf);
    }
    template void serialize( boost::mpi::packed_skeleton_iarchive&, Kernel::InfectionTyphoid&, unsigned int);
}
#endif

#endif // ENABLE_TYPHOID
