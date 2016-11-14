/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "TyphoidVaccine.h"
#include "InterventionsContainer.h" 
#include "NodeEventContext.h"
#include "Individual.h"

static const char* _module = "TyphoidVaccine";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(TyphoidVaccine)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        //HANDLE_INTERFACE(IVaccine)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
    END_QUERY_INTERFACE_BODY(TyphoidVaccine)

    IMPLEMENT_FACTORY_REGISTERED(TyphoidVaccine)

    bool
    TyphoidVaccine::Configure(
        const Configuration * inputJson
    )
    {
        initConfig( "Mode", vaccine_mode, inputJson, MetadataDescriptor::Enum("Mode", "Shedding, Dose, or Exposures", MDD_ENUM_ARGS(TyphoidVaccineMode)) );
        // not sure whether we're using route yet.
        initConfig( "Route", route, inputJson, MetadataDescriptor::Enum("Route", "Contact or Environmental", MDD_ENUM_ARGS(TransmissionRoute)) );
        initConfigTypeMap("Effect", &effect, "How effective is this?", 0.0, 1.0, 1.0 ); 
        initConfigComplexType("Changing_Effect", &changing_config, "Highly configurable effect that changes over time, derived from WaningConfig." );

        bool configured = BaseIntervention::Configure( inputJson );
        if( !JsonConfigurable::_dryrun )
        {
            if( route != TransmissionRoute::TRANSMISSIONROUTE_CONTACT &&
                route != TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL )
            {
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "The 'Route' param can only be CONTACT or ENVIRONMENT in this intervention." );
            }
            auto tmp_waning = Configuration::CopyFromElement( changing_config._json );
            // Would really like to find the right way to see if user omitted the Changing_Config instead of using exception handling.
            try {
                changing_effect = WaningEffectFactory::CreateInstance( tmp_waning );
            }
            catch( JsonTypeConfigurationException e )
            {
                LOG_INFO_F( "Looks like we're just going with fixed-value effect and not a variable-over-time effect structure.\n" );
            }
            delete tmp_waning;
            tmp_waning = nullptr;
            
            //changing_effect->SetCurrentTime( parent->GetEventContext()->GetNodeEventContext()->GetTime().time );
        }
        LOG_DEBUG_F( "Vaccine configured with type %d and effect %f.\n", vaccine_mode, effect );
        return configured;
    }

    TyphoidVaccine::TyphoidVaccine() 
    : changing_effect( nullptr )
    , route( TransmissionRoute::TRANSMISSIONROUTE_CONTACT )
    , effect( 1.0f )
    , vaccine_mode( TyphoidVaccineMode::Shedding )
    {
    }

    TyphoidVaccine::TyphoidVaccine( const TyphoidVaccine& master )
    : changing_config(master.changing_config)
    , changing_effect( nullptr )
    {
        vaccine_mode = master.vaccine_mode;
        route = master.route;
        effect = master.effect;
        auto tmp_changing = Configuration::CopyFromElement( changing_config._json );
        try {
            changing_effect = WaningEffectFactory::CreateInstance( tmp_changing );
        }
        catch( JsonTypeConfigurationException e )
        {
            LOG_INFO_F( "Looks like we're just going with fixed-value effect and not a variable-over-time effect structure.\n" );
        }
        delete tmp_changing;
        tmp_changing = nullptr;
    }

    TyphoidVaccine::~TyphoidVaccine()
    {
        delete changing_effect;
        changing_effect = nullptr;
    }

    bool
    TyphoidVaccine::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * pCCO
    )
    {
        // store itvc for apply
        LOG_DEBUG("Distributing SimpleVaccine.\n");
        if (s_OK != context->QueryInterface(GET_IID(ITyphoidVaccineEffectsApply), (void**)&itvc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "ITyphoidVaccineEffectsApply", "IIndividualHumanInterventionsContext" );
        }

        /*auto iface = static_cast<InterventionsContainer*>(context);
        auto iface2 = static_cast<TyphoidInterventionsContainer*>(iface);
        itvc = static_cast<ITyphoidVaccineEffectsApply*>(iface2);*/

        bool distribute =  BaseIntervention::Distribute( context, pCCO );
        
        //changing_effect->SetCurrentTime( context->GetEventContext()->GetNodeEventContext()->GetTime().time );
        changing_effect->SetCurrentTime( ((IndividualHuman*)(context->GetParent()))->GetParent()->GetTime().time );
        return distribute;
    }

    void TyphoidVaccine::Update( float dt )
    {
        release_assert(itvc);
        auto _effect = effect; // this is bad and confusing. Don't do this. Talking to myself here...
        if( changing_effect )
        {
            changing_effect->Update( dt );
            _effect = changing_effect->Current();
        }
        auto multiplier = 1.0f-_effect;
        switch( vaccine_mode )
        {
            case TyphoidVaccineMode::Shedding:
            itvc->ApplyReducedSheddingEffect( multiplier, route );
            break;

            case TyphoidVaccineMode::Dose:
            itvc->ApplyReducedDoseEffect( multiplier, route );
            break;
        
            case TyphoidVaccineMode::Exposures:
            itvc->ApplyReducedNumberExposuresEffect( multiplier, route );
            break;

            default:
            throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "vaccine_mode", vaccine_mode, TyphoidVaccineMode::pairs::lookup_key( vaccine_mode ) );
        }
    }

    void TyphoidVaccine::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        parent = context;
        if (s_OK != parent->GetInterventionsContext()->QueryInterface(GET_IID(ITyphoidVaccineEffectsApply), (void**)&itvc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context->GetInterventionsContext()", "ITyphoidVaccineEffectsApply", "IIndividualHumanInterventionsContext" );
        }
        release_assert( parent );
        release_assert( parent->GetEventContext() );
        release_assert( parent->GetEventContext()->GetNodeEventContext() );
        changing_effect->SetCurrentTime( parent->GetEventContext()->GetNodeEventContext()->GetTime().time );
        //LOG_DEBUG_F( "Vaccine configured with type %d and take %f for individual %d\n", vaccine_type, vaccine_take, parent->GetSuid().data );
    } // needed for VaccineTake

    REGISTER_SERIALIZABLE(TyphoidVaccine);

    void TyphoidVaccine::serialize(IArchive& ar, TyphoidVaccine* obj)
    {
        BaseIntervention::serialize( ar, obj );
        TyphoidVaccine& vaccine = *obj;
        //ar.labelElement("acquire_effect")                 & vaccine.acquire_effect;
    }
}
