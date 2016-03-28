/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "PolioDrugTypeParameters.h"

#include "Exceptions.h"
#include <algorithm> // for transform
#include <cctype> // for tolower

static const char * _module = "PolioDrugTypeParameters";

std::map< int, Kernel::PolioDrugTypeParameters* > Kernel::PolioDrugTypeParameters::_pdtMap;

namespace Kernel
{
    PolioDrugTypeParameters::PolioDrugTypeParameters(
        const PolioDrugType::Enum &drugType
    )
    { LOG_DEBUG( "ctor\n" );
    }

    PolioDrugTypeParameters::~PolioDrugTypeParameters()
    { LOG_DEBUG( "dtor\n" );
    }

    PolioDrugTypeParameters* PolioDrugTypeParameters::CreatePolioDrugTypeParameters(
        const PolioDrugType::Enum &drugType
    )
    { 
        LOG_DEBUG( "Create\n" );

        PolioDrugTypeParameters* params = NULL;
        map< int, PolioDrugTypeParameters* >::const_iterator itMap = _pdtMap.find(drugType);

        if ( itMap == _pdtMap.end() )
        {
            params = _new_ PolioDrugTypeParameters( drugType );
            _pdtMap[ drugType ] = params;
            return params;
        }
        else
        {
            return itMap->second;
        }
    }

    bool
    PolioDrugTypeParameters::Configure(
        const ::Configuration *config
    )
    {
        LOG_DEBUG( "Configure\n" );

        return JsonConfigurable::Configure( config );
    }

    QueryResult
    PolioDrugTypeParameters::QueryInterface(
        iid_t iid, void **ppvObject
    )
    {
        throw NotYetImplementedException(  __FILE__, __LINE__, __FUNCTION__ );
    }
}

