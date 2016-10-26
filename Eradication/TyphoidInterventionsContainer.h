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

#ifdef ENABLE_TYPHOID
#include <string>
#include <list>
#include <vector>

#include "Drugs.h"
#include "Interventions.h"
#include "InterventionsContainer.h"

namespace Kernel
{
    // this container becomes a help implementation member of the relevant IndividualHuman class 
    // it needs to implement consumer interfaces for all the relevant intervention types
    struct ITyphoidDrugEffectsApply : public ISupports
    {
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ) = 0;
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ) = 0;
    };

    struct ITyphoidVaccineEffectsApply : public ISupports
    {
        virtual void ApplyReducedSheddingEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) = 0;
        virtual void ApplyReducedDoseEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) = 0;
        virtual void ApplyReducedNumberExposuresEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) = 0;
    };

    class ITyphoidVaccine;

    class TyphoidInterventionsContainer : public InterventionsContainer,
                                          public ITyphoidVaccineEffectsApply,
                                          public ITyphoidDrugEffectsApply
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        TyphoidInterventionsContainer();
        virtual ~TyphoidInterventionsContainer();

        virtual QueryResult QueryInterface(iid_t iid, void** pinstance);

        // IVaccineConsumer: not any more!
        virtual bool GiveIntervention( IDistributableIntervention * pIV );

        // ITyphoidDrugEffectsApply
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ); // not used for anything
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ); // not used for anything

        // Typhoid 'Vaccine' Apply/Update/Setter functions
        virtual void ApplyReducedSheddingEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) override; 
        virtual void ApplyReducedDoseEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) override;
        virtual void ApplyReducedNumberExposuresEffect( float rate, const TransmissionRoute::Enum &route = TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ) override;

        // Typhoid 'Vaccine' Getter functions, x6, explicit
        virtual float GetContactDepositAttenuation() const
        {
            return current_shedding_attenuation_contact;
        }

        virtual float GetEnviroDepositAttenuation() const
        {
            return current_shedding_attenuation_environment;
        }

        virtual float GetContactExposuresAttenuation() const
        {
            return current_exposures_attenuation_contact;
        }

        virtual float GetEnviroExposuresAttenuation() const
        {
            return current_exposures_attenuation_environment;
        }

        virtual void Update(float dt); // example of intervention timestep update

    protected:
        void GiveDrug(IDrug* drug);
        float current_shedding_attenuation_contact;
        float current_shedding_attenuation_environment;
        float current_dose_attenuation_contact;
        float current_dose_attenuation_environment;
        float current_exposures_attenuation_contact;
        float current_exposures_attenuation_environment;

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class ::boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, TyphoidInterventionsContainer& cont, const unsigned int v);
#endif
    };
}
#endif
