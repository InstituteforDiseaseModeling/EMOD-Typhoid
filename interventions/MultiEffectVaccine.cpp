/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "MultiEffectVaccine.h"
#include "InterventionsContainer.h"  // for IVaccineConsumer methods

static const char* _module = "MultiEffectVaccine";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(MultiEffectVaccine)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(IVaccine)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
    END_QUERY_INTERFACE_BODY(MultiEffectVaccine)

    IMPLEMENT_FACTORY_REGISTERED(MultiEffectVaccine)

    bool
    MultiEffectVaccine::Configure(
        const Configuration * inputJson
    )
    {
        initConfigTypeMap("Vaccine_Take", &vaccine_take, SV_Vaccine_Take_DESC_TEXT, 0.0, 1.0, 1.0 ); 
        initConfigComplexType("Acquire_Config",  &acquire_config, "TBD" );
        initConfigComplexType("Transmit_Config",  &transmit_config, "TBD" );
        initConfigComplexType("Mortality_Config", &mortality_config, "TBD" );

        bool configured = BaseIntervention::Configure( inputJson );
        if( !JsonConfigurable::_dryrun )
        {
            acquire_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( acquire_config._json ) );
            transmit_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( transmit_config._json ) );
            mortality_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( mortality_config._json ) );
        }
        //release_assert( vaccine_type );
        LOG_DEBUG_F( "Vaccine configured with type %d and take %f.\n", vaccine_type, vaccine_take );
        return configured;
    }

    MultiEffectVaccine::MultiEffectVaccine() 
    : SimpleVaccine()
    , acquire_effect( nullptr )
    , transmit_effect( nullptr )
    , mortality_effect( nullptr )
    {
    }

    MultiEffectVaccine::MultiEffectVaccine( const MultiEffectVaccine& master )
    //: BaseIntervention( master )
    {
        vaccine_take = master.vaccine_take;
        cost_per_unit = master.cost_per_unit;
        acquire_config = master.acquire_config;
        transmit_config = master.transmit_config;
        mortality_config = master.mortality_config;

        // dupe code
        /*switch( vaccine_type )
        {
            case SimpleVaccineType::AcquisitionBlocking:
                acquire_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( acquire_config._json ) );
                break;

            case SimpleVaccineType::TransmissionBlocking:
                transmit_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( transmit_config._json ) );
                break;

            case SimpleVaccineType::MortalityBlocking:
                mortality_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( mortality_config._json ) );
                break;

            case SimpleVaccineType::Generic:*/
                acquire_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( acquire_config._json ) );
                transmit_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( transmit_config._json ) );
                mortality_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( mortality_config._json ) );
                //break;
        //}

    }

    MultiEffectVaccine::~MultiEffectVaccine() { }

    void MultiEffectVaccine::Update( float dt )
    {
        release_assert(ivc);

        /*switch( vaccine_type )
        {
            case SimpleVaccineType::AcquisitionBlocking:
                acquire_effect->Update(dt);
                current_reducedacquire = acquire_effect->Current();
                ivc->UpdateVaccineAcquireRate( current_reducedacquire );
                break;

            case SimpleVaccineType::TransmissionBlocking:
                transmit_effect->Update(dt);
                current_reducedtransmit  = transmit_effect->Current();
                ivc->UpdateVaccineTransmitRate( current_reducedtransmit );
                break;

            case SimpleVaccineType::MortalityBlocking:
                mortality_effect->Update(dt);
                current_reducedmortality  = mortality_effect->Current(); 
                ivc->UpdateVaccineMortalityRate( current_reducedmortality );
                break;

            case SimpleVaccineType::Generic:*/
                acquire_effect->Update(dt);
                current_reducedacquire = acquire_effect->Current();
                ivc->UpdateVaccineAcquireRate( current_reducedacquire );

                transmit_effect->Update(dt);
                current_reducedtransmit  = transmit_effect->Current();
                ivc->UpdateVaccineTransmitRate( current_reducedtransmit );

                mortality_effect->Update(dt);
                current_reducedmortality  = mortality_effect->Current(); 
                ivc->UpdateVaccineMortalityRate( current_reducedmortality );
                /*break;

            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "vaccine_type", vaccine_type, SimpleVaccineType::pairs::lookup_key( vaccine_type ) );
                break;*/
        //}
    }


    REGISTER_SERIALIZABLE(MultiEffectVaccine);

    void MultiEffectVaccine::serialize(IArchive& ar, MultiEffectVaccine* obj)
    {
        SimpleVaccine::serialize( ar, obj );
        MultiEffectVaccine& vaccine = *obj;
        ar.labelElement("acquire_effect")                 & vaccine.acquire_effect;
        ar.labelElement("transmit_effect")                 & vaccine.transmit_effect;
        ar.labelElement("mortality_effect")                 & vaccine.mortality_effect;
    }
}
