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
#include "PyDemoDrugTypeParameters.h"

#include "Exceptions.h"
#include <algorithm> // for transform
#include <cctype> // for tolower

static const char * _module = "PyDemoDrugTypeParameters";

std::map< int, Kernel::PyDemoDrugTypeParameters* > Kernel::PyDemoDrugTypeParameters::_pdtMap;

namespace Kernel
{
    PyDemoDrugTypeParameters::PyDemoDrugTypeParameters(
        const PyDemoDrugType::Enum &drugType
    )
    { LOG_DEBUG( "ctor\n" );
    }

    PyDemoDrugTypeParameters::~PyDemoDrugTypeParameters()
    { LOG_DEBUG( "dtor\n" );
    }

    PyDemoDrugTypeParameters* PyDemoDrugTypeParameters::CreatePyDemoDrugTypeParameters(
        const PyDemoDrugType::Enum &drugType
    )
    { 
        LOG_DEBUG( "Create\n" );

        PyDemoDrugTypeParameters* params = NULL;
        map< int, PyDemoDrugTypeParameters* >::const_iterator itMap = _pdtMap.find(drugType);

        if ( itMap == _pdtMap.end() )
        {
            params = _new_ PyDemoDrugTypeParameters( drugType );
            _pdtMap[ drugType ] = params;
            return params;
        }
        else
        {
            return itMap->second;
        }
    }

    bool
    PyDemoDrugTypeParameters::Configure(
        const ::Configuration *config
    )
    {
        LOG_DEBUG( "Configure\n" );

        return JsonConfigurable::Configure( config );
    }

    QueryResult
    PyDemoDrugTypeParameters::QueryInterface(
        iid_t iid, void **ppvObject
    )
    {
        throw NotYetImplementedException(  __FILE__, __LINE__, __FUNCTION__ );
    }
}

