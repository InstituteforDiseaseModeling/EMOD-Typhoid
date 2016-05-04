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

#pragma once
#if 0
#include "Configure.h"
#include "InterventionEnums.h"
#include "TyphoidDrugTypeParameters.h"

namespace Kernel {
    class TyphoidDrugTypeParameters : public JsonConfigurable
    {
        friend class SimulationConfig;
        friend class AntipoliovirusDrug;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
    public:
        TyphoidDrugTypeParameters( const TyphoidDrugType::Enum& drugType );
        static TyphoidDrugTypeParameters* CreateTyphoidDrugTypeParameters( const TyphoidDrugType::Enum& drugType );
        virtual ~TyphoidDrugTypeParameters();
        bool Configure( const ::Configuration *json );
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);

        static map< int, TyphoidDrugTypeParameters* > _pdtMap;


    protected:

    private:
        TyphoidDrugType::Enum _drugType;
    };
}
#endif