/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Configure.h"

namespace Kernel
{
    ENUM_DEFINE(PolioDrugType,
        ENUM_VALUE_SPEC(V073                    , 1)
        ENUM_VALUE_SPEC(Combo                   , 2))

    class PolioDrugTypeParameters : public JsonConfigurable
    {
        friend class SimulationConfig;
        friend class AntipoliovirusDrug;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
    public:
        PolioDrugTypeParameters( const PolioDrugType::Enum& drugType );
        static PolioDrugTypeParameters* CreatePolioDrugTypeParameters( const PolioDrugType::Enum& drugType );
        virtual ~PolioDrugTypeParameters();
        bool Configure( const ::Configuration *json );
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);

        static map< int, PolioDrugTypeParameters* > _pdtMap;


    protected:

    private:
        PolioDrugType::Enum _drugType;
    };
}
