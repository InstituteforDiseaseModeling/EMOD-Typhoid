/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "SimpleImmunoglobulin.h"
#include "Common.h"             // for INFINITE_TIME
#include "InterventionsContainer.h"

static const char * _module = "SimpleImmunoglobulin";

namespace Kernel
{
    ENUM_DEFINE(ImmunoglobulinType,
        ENUM_VALUE_SPEC(StrainSpecific      , 1)
        ENUM_VALUE_SPEC(BroadlyNeutralizing , 2))

    IMPLEMENT_FACTORY_REGISTERED(SimpleImmunoglobulin)

    SimpleImmunoglobulin::SimpleImmunoglobulin()
    {
        // done in base class ctor, not needed here.
        // primary_decay_time_constant...
        // cost_per_unit...
    }

    SimpleImmunoglobulin::SimpleImmunoglobulin( const SimpleImmunoglobulin& master )
    {
        vaccine_type = master.vaccine_type;
        vaccine_take = master.vaccine_take;
        cost_per_unit = master.cost_per_unit;
        acquire_config = master.acquire_config;
        transmit_config = master.transmit_config;

        auto tmp_acquire   = Configuration::CopyFromElement( acquire_config._json   );
        auto tmp_transmit  = Configuration::CopyFromElement( transmit_config._json  );

        acquire_effect   = WaningEffectFactory::CreateInstance( tmp_acquire   );
        transmit_effect  = WaningEffectFactory::CreateInstance( tmp_transmit  );

        delete tmp_acquire;
        delete tmp_transmit;
        tmp_acquire   = nullptr;
        tmp_transmit  = nullptr;
    }

#define SI_Acquire_Config_DESC_TEXT  "TBD"
#define SI_Transmit_Config_DESC_TEXT  "TBD"

    bool
    SimpleImmunoglobulin::Configure(
        const Configuration * inputJson
    )
    {
        vaccine_take = 1.0; // immunoglobulin always takes in the model 

        initConfig( "Vaccine_Type", vaccine_type, inputJson, MetadataDescriptor::Enum("immunoglobulin_type", SI_Vaccine_Type_DESC_TEXT, MDD_ENUM_ARGS(ImmunoglobulinType))); // required? 
    
        initConfigComplexType("Acquire_Config",  &acquire_config, SI_Acquire_Config_DESC_TEXT );
        initConfigComplexType("Transmit_Config",  &transmit_config, SI_Transmit_Config_DESC_TEXT  );
        bool configured = BaseIntervention::Configure( inputJson );
        if( !JsonConfigurable::_dryrun )
        {
            auto tmp_acquire   = Configuration::CopyFromElement( acquire_config._json   );
            auto tmp_transmit  = Configuration::CopyFromElement( transmit_config._json  );

            acquire_effect   = WaningEffectFactory::CreateInstance( tmp_acquire   );
            transmit_effect  = WaningEffectFactory::CreateInstance( tmp_transmit  );

            delete tmp_acquire;
            delete tmp_transmit;
            tmp_acquire   = nullptr;
            tmp_transmit  = nullptr;
        }

        return configured;
    }

    void SimpleImmunoglobulin::Update( float dt )
    {
        acquire_effect->Update(dt);
        current_reducedacquire = acquire_effect->Current();
        ivc->UpdateVaccineAcquireRate( current_reducedacquire );

        transmit_effect->Update(dt);
        current_reducedtransmit = transmit_effect->Current();
        ivc->UpdateVaccineTransmitRate( current_reducedtransmit );
    }

    REGISTER_SERIALIZABLE(SimpleImmunoglobulin);

    void SimpleImmunoglobulin::serialize(IArchive& ar, SimpleImmunoglobulin* obj)
    {
        SimpleVaccine::serialize( ar, obj );
        SimpleImmunoglobulin& simple = *obj;

        ar.labelElement("acquire_effect") & simple.acquire_effect;
        ar.labelElement("transmit_effect") & simple.transmit_effect;
    }
}

