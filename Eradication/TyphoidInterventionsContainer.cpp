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
#ifdef ENABLE_TYPHOID

#include "Sugar.h"
#include "Debug.h"
#include "Environment.h"

#include "Drugs.h"

#include "Contexts.h"
#include "InterventionFactory.h"
#include "TyphoidInterventionsContainer.h"


namespace Kernel
{
    static const char* _module = "TyphoidInterventionsContainer";

    BEGIN_QUERY_INTERFACE_DERIVED(TyphoidInterventionsContainer, InterventionsContainer)
        HANDLE_INTERFACE(ITyphoidVaccineEffectsApply)
    END_QUERY_INTERFACE_DERIVED(TyphoidInterventionsContainer, InterventionsContainer)

    TyphoidInterventionsContainer::TyphoidInterventionsContainer()
        : current_shedding_attenuation_contact( 1.0f )
        , current_dose_attenuation_contact( 1.0f )
        , current_exposures_attenuation_contact( 1.0f )
        , current_shedding_attenuation_environment( 1.0f )
        , current_dose_attenuation_environment( 1.0f )
        , current_exposures_attenuation_environment( 1.0f )
    {
    }

    TyphoidInterventionsContainer::~TyphoidInterventionsContainer()
    {
    }

    void TyphoidInterventionsContainer::Update(float dt)
    {
        // call base level
        InterventionsContainer::Update(dt);
    }

    bool TyphoidInterventionsContainer::GiveIntervention( IDistributableIntervention * pIV )
    {
        pIV->SetContextTo( parent );

        // Additionally, keep this newly-distributed polio-vaccine pointer in the 'new_vaccines' list.  
        // IndividualHuman::applyNewInterventionEffects will come looking for these to apply to SusceptibilityTyphoid.
        IDistributableIntervention* ipvac = nullptr;
/*
        IDrug * pDrug = NULL;
        if( s_OK == pIV->QueryInterface(GET_IID(IDrug), (void**) &pDrug) )
        {
            LOG_DEBUG("Getting a drug\n");
            GiveDrug( pDrug );
        } 
*/
        return InterventionsContainer::GiveIntervention(pIV);
    }

    void TyphoidInterventionsContainer::GiveDrug(IDrug* drug)
    {
        drug->ConfigureDrugTreatment();
    }

    void TyphoidInterventionsContainer::ApplyReducedSheddingEffect( float rate, const TransmissionRoute::Enum &route )
    {
        LOG_VALID_F( "%s\n", __FUNCTION__ );
        if( route == TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL )
        {
            LOG_VALID_F( "%s: Set current_shedding_attenuation_environment  to %f for individual %d.\n", __FUNCTION__, rate, parent->GetSuid().data );
            current_shedding_attenuation_environment = rate;
        }
        else // CONTACT
        {
            LOG_VALID_F( "%s: Set current_shedding_attenuation_contact to %f for individual %d.\n", __FUNCTION__, rate, parent->GetSuid().data );
            current_shedding_attenuation_contact = rate;
        }
    }

    void TyphoidInterventionsContainer::ApplyReducedDoseEffect( float rate, const TransmissionRoute::Enum &route )
    {
        LOG_VALID_F( "%s\n", __FUNCTION__ );
        if( route == TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL )
        {
            LOG_VALID_F( "%s: Set current_dose_attenuation_environment to %f for individual %d.\n", __FUNCTION__, rate, parent->GetSuid().data );
            current_dose_attenuation_environment = rate;
        }
        else // CONTACT
        {
            LOG_VALID_F( "%s: Set current_dose_attenuation_contact to %f for individual %d.\n", __FUNCTION__, rate, parent->GetSuid().data );
            current_dose_attenuation_contact = rate;
        }
    }

    void TyphoidInterventionsContainer::ApplyReducedNumberExposuresEffect( float rate, const TransmissionRoute::Enum &route )
    {
        if( route == TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL )
        {
            current_exposures_attenuation_environment = rate;
            LOG_VALID_F( "%s: Set current_exposures_attenuation_environment to %f for individual %d.\n", __FUNCTION__, rate, parent->GetSuid().data );
        }
        else // CONTACT
        {
            LOG_VALID_F( "%s: Set current_exposures_attenuation_contact to %f for individual %d.\n", __FUNCTION__, rate, parent->GetSuid().data );
            current_exposures_attenuation_contact = rate;
        }
    }

    float TyphoidInterventionsContainer::GetContactDepositAttenuation() const
    {
        LOG_VALID_F( "%s: Returning %f for current_shedding_attenuation_contact for individual %d.\n", __FUNCTION__, current_shedding_attenuation_contact, parent->GetSuid().data );
        return current_shedding_attenuation_contact;
    }

    float TyphoidInterventionsContainer::GetEnviroDepositAttenuation() const
    {
        LOG_VALID_F( "%s: Returning %f for current_shedding_attenuation_environment for individual %d.\n", __FUNCTION__, current_shedding_attenuation_environment, parent->GetSuid().data );
        return current_shedding_attenuation_environment;
    }

    float TyphoidInterventionsContainer::GetContactExposuresAttenuation() const
    {
        return current_exposures_attenuation_contact;
    }

    float TyphoidInterventionsContainer::GetEnviroExposuresAttenuation() const
    {
        return current_exposures_attenuation_environment;
    }

}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::TyphoidInterventionsContainer)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive &ar, TyphoidInterventionsContainer& cont, const unsigned int v)
    {
        static const char * _module = "TyphoidInterventionsContainer";
        LOG_DEBUG("(De)serializing TyphoidInterventionsContainer\n");

        //ar & cont.new_vaccines;  // SusceptibilityTyphoid update based on new vaccines done in same time step?
        boost::serialization::void_cast_register<TyphoidInterventionsContainer, InterventionsContainer>();
        ar & boost::serialization::base_object<Kernel::InterventionsContainer>(cont);
    }
}
#endif

#endif // ENABLE_TYPHOID
