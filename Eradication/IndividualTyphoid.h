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

#include "Contexts.h"
#include "InfectionTyphoid.h"
#include "IndividualEnvironmental.h"
#include "SusceptibilityTyphoid.h"

namespace Kernel
{
    class SusceptibilityTyphoid;
    class IndividualHumanTyphoidConfig : public JsonConfigurable
    {
        GET_SCHEMA_STATIC_WRAPPER(IndividualHumanTyphoidConfig)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    protected:
        friend class SimulationTyphoid;
        friend class IndividualHumanTyphoid;
        friend class InfectionTyphoid;
        friend class SusceptibilityTyphoid;
        static float environmental_incubation_period; // NaturalNumber please
        static float typhoid_acute_infectiousness;
        static float typhoid_chronic_relative_infectiousness;

//        static float typhoid_environmental_exposure;
        static float typhoid_prepatent_relative_infectiousness;
        static float typhoid_protection_per_infection;
        static float typhoid_subclinical_relative_infectiousness;
        static float typhoid_carrier_probability_male;
		static float typhoid_carrier_probability_female;
        static float typhoid_3year_susceptible_fraction;
        static float typhoid_6month_susceptible_fraction;
        static float typhoid_6year_susceptible_fraction;
		static float typhoid_symptomatic_fraction;

        static float typhoid_environmental_exposure_rate;
        static float typhoid_contact_exposure_rate;
        //static float typhoid_environmental_exposure_rate_seasonal_max;
        static float typhoid_environmental_ramp_up_duration;
		static float typhoid_environmental_ramp_down_duration;
		static float typhoid_environmental_peak_start;
        static float typhoid_environmental_cutoff_days;
        //static float typhoid_environmental_amplification;
        static float typhoid_environmental_peak_multiplier;

        virtual bool Configure( const Configuration* config );
    };


    class IIndividualHumanTyphoid : public ISupports
    {
    public:
        virtual bool IsChronicCarrier( bool incidence_only = true ) const = 0;
        virtual bool IsSubClinical( bool incidence_only = true ) const = 0;
        virtual bool IsAcute( bool incidence_only = true ) const = 0;
        virtual bool IsPrePatent( bool incidence_only = true ) const = 0;
    };

    class IndividualHumanTyphoid : public IndividualHumanEnvironmental, public IIndividualHumanTyphoid
    {
        friend class SimulationTyphoid;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE()
        DECLARE_SERIALIZABLE(IndividualHumanTyphoid);

    public:
        static IndividualHumanTyphoid *CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual ~IndividualHumanTyphoid(void);
        static void InitializeStatics( const Configuration* config );

        virtual void CreateSusceptibility(float imm_mod = 1.0, float risk_mod = 1.0);
        virtual void ExposeToInfectivity(float dt = 1.0, const TransmissionGroupMembership_t* transmissionGroupMembership = NULL);

        virtual bool IsChronicCarrier( bool incidence_only = true ) const;
        virtual bool IsSubClinical( bool incidence_only = true ) const;
        virtual bool IsAcute( bool incidence_only = true ) const;
        virtual bool IsPrePatent( bool incidence_only = true ) const;
        const std::string getDoseTracking() const;

        // New Exposure Pattern
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route );

        IndividualHumanTyphoid(suids::suid id = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual void setupInterventionsContainer();
        virtual void PropagateContextToDependents();

        virtual void UpdateInfectiousness(float dt);
        virtual void Update(float currenttime, float dt);
        virtual Infection* createInfection(suids::suid _suid);
        virtual void AcquireNewInfection(StrainIdentity *infstrain = NULL, int incubation_period_override = -1);
        virtual float GetImmunityReducedAcquire() const override;
        virtual HumanStateChange GetStateChange() const;
        
    protected:
        float getSeasonalAmplitude() const;
        void quantizeContactDoseTracking( float fContact );
        void quantizeEnvironmentalDoseTracking( float fEnvironment );

        std::string processPrePatent( float dt );

        // typhoid infection state
        std::string state_to_report; // typhoid status of individual: cache from infection
        bool isChronic;       // is or will be a chronic carrier (using "Ames" definition)
        int _infection_count;     // number of times infected;
        TransmissionRoute::Enum _routeOfInfection; // how did this person get infected?
        bool state_changed;
        std::string doseTracking;



        // typhoid constants
        static const float P1; // probability that an infection becomes clinical
        static const float P5; // probability of typhoid death
        //////////JG REMOVE static const float P6; // probability of sterile immunity after acute infection
        static const float P7; // probability of clinical immunity after acute infection
        //////////JG REMOVE static const float P8; // probability of sterile immunity from a subclinical infectin in the clinically immune
        //////////JG REMOVE static const float P9; // probability of sterile immunity from a subclinical infection


        // typhoid constants from "OutBase.csv" file
        //////////JG REMOVE static float agechronicmale[200]; //probability of becoming chronic carrier, male
        //////////JG REMOVE static float agechronicfemale[200]; //probability of becoming chronic carrier, female

        // environmental exposure constants
        static const int N50;
        static const float alpha;

    private:
        SusceptibilityTyphoid * typhoid_susceptibility;
        std::map< TransmissionRoute::Enum, float > contagion_population_by_route;
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        ///////////////////////////////////////////////////////////////////////////
        // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, IndividualHumanTyphoid& flags, const unsigned int  file_version );

        ////////////////////////////////////////////////////////////////////////////
#endif
    };
}
