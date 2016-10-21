/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "TyphoidVaccine.h"
#include "InterventionsContainer.h"  // for IVaccineConsumer methods

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

        bool configured = BaseIntervention::Configure( inputJson );
        if( !JsonConfigurable::_dryrun )
        {
        }
        LOG_DEBUG_F( "Vaccine configured with type %d and effect %f.\n", vaccine_mode, effect );
        return configured;
    }

    TyphoidVaccine::TyphoidVaccine() 
    {
    }

    TyphoidVaccine::TyphoidVaccine( const TyphoidVaccine& master )
    {
        //vaccine_take = master.vaccine_take; 
    }

    TyphoidVaccine::~TyphoidVaccine()
    {
    }

    bool
    TyphoidVaccine::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * pCCO
    )
    {
        // store itvc for apply
        LOG_DEBUG("Distributing SimpleVaccine.\n");
        if (s_OK != context->QueryInterface(GET_IID(IVaccineConsumer), (void**)&itvc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IVaccineConsumer", "IIndividualHumanInterventionsContext" );
        }

        bool distribute =  BaseIntervention::Distribute( context, pCCO );
        if( vaccine_mode == TyphoidVaccineMode::Shedding )
        {
            itvc->ApplyReducedSheddingEffect( effect );
        }
        else if( vaccine_mode == TyphoidVaccineMode::Dose )
        {
            itvc->ApplyReducedDoseEffect( effect );
        }
        else if( vaccine_mode == TyphoidVaccineMode::Exposures )
        {
            itvc->ApplyReducedNumberExposuresEffect( effect );
        }
        return distribute;
    }

    void TyphoidVaccine::Update( float dt )
    {
        release_assert(itvc);

    }

    void TyphoidVaccine::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        parent = context;
        if (s_OK != parent->GetInterventionsContext()->QueryInterface(GET_IID(IVaccineConsumer), (void**)&itvc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context->GetInterventionsContext()", "IVaccineConsumer", "IIndividualHumanInterventionsContext" );
        }
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
