/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "InterventionEnums.h"
#include "BasicTestNodeTargetedIntervention.h"
#include "InterventionFactory.h"
#include "ConfigurationImpl.h"
#include "NodeEventContext.h"

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(SimpleBTNTI)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_INTERFACE(IBTNTI)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleBTNTI)

    IMPLEMENT_SERIALIZABLE(SimpleBTNTI)

    REGISTER_SERIALIZATION_VOID_CAST(SimpleBTNTI, INodeDistributableIntervention)

    IMPLEMENT_FACTORY_REGISTERED(SimpleBTNTI)

    SimpleBTNTI::SimpleBTNTI()
    {
        initConfigTypeMap( "efficacy", &efficacy, BTNTI_Efficacy_DESC_TEXT, 0.0, 1.0, 1.0 );
    }

    bool
    SimpleBTNTI::Configure(
        const Configuration* inputJson
    )
    {
        return JsonConfigurable::Configure( inputJson );
    }

    // Will move to BaseIntervention soon
    json::QuickBuilder
    SimpleBTNTI::GetSchema()
    {
        return json::QuickBuilder( jsonSchemaBase );
    }

    void SimpleBTNTI::Update( float dt )
    {
    }
}
