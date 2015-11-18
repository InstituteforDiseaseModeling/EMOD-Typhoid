/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "NodeSet.h"

static const char* _module = "NodeSetAll";

namespace Kernel
{
    // NodeSetAll
    IMPLEMENT_FACTORY_REGISTERED(NodeSetAll)
    IMPLEMENT_FACTORY_REGISTERED(NodeSetNodeList)
    IMPLEMENT_FACTORY_REGISTERED(NodeSetPolygon)

    IMPL_QUERY_INTERFACE2(NodeSetAll, INodeSet, IConfigurable)

    json::QuickBuilder
    NodeSetAll::GetSchema()
    //const
    {
        return json::QuickBuilder( jsonSchemaBase );
    }

    bool
    NodeSetAll::Configure(
        const Configuration * pInputJson
    )
    {
        return true; // nothing to configure
    }

    bool
    NodeSetAll::Contains(
        INodeEventContext *ndc
    )
    {
        return true;
    }
}
