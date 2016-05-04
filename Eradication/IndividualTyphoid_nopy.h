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
        GET_SCHEMA_STATIC_WRAPPER(IndividualHumanTyphoid);

    public:
        static IndividualHumanTyphoid *CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual ~IndividualHumanTyphoid(void) {}

        virtual void CreateSusceptibility(float imm_mod = 1.0, float risk_mod = 1.0);
        virtual void ExposeToInfectivity(float dt = 1.0, const TransmissionGroupMembership_t* transmissionGroupMembership = NULL);

        virtual bool IsChronicCarrier( bool incidence_only = true ) const;
        virtual bool IsSubClinical( bool incidence_only = true ) const;
        virtual bool IsAcute( bool incidence_only = true ) const;
        virtual bool IsPrePatent( bool incidence_only = true ) const;

    protected:

        // New Exposure Pattern
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route );

        IndividualHumanTyphoid(suids::suid id = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual void setupInterventionsContainer();
        virtual void PropagateContextToDependents();

        virtual void UpdateInfectiousness(float dt);
        virtual void Update(float currenttime, float dt);
        virtual Infection* createInfection(suids::suid _suid);
        virtual void AcquireNewInfection(StrainIdentity *infstrain = NULL, int incubation_period_override = -1);
        virtual HumanStateChange GetStateChange() const;

        std::string processPrePatent( float dt );

        // typhoid infection state
        std::string state_to_report; // typhoid status of individual
        std::string last_state_reported; // previous typhoid status of individual
        int chronic_timer,
            subclinical_timer,
            acute_timer,
            prepatent_timer,
            infectious_timer,
            //////////JG REMOVE  sterile_immunity_timer,
            clinical_immunity_timer;  // timers of days left in state, or -1 if not used //JG- I'm going to leave clinical immunity in for now. 
        int _subclinical_duration,
            _prepatent_duration,
            _acute_duration; // duration of state in days
        //////////JG REMOVE bool hasSterileImmunity;  // is completely immune to infection // No more sterile immunity
        bool hasClinicalImmunity; // is immune to clinical infection
        bool isAmesChronic;       // is a chronic carrier //JG- we are permanently using "Ames Chronic" style carriers
        int _infection_count;     // number of times infected;
        TransmissionRoute::Enum _routeOfInfection; // how did this person get infected?
        bool isDead;  // is this individual dead?

        // typhoid constants
        static const double P1; // probability that an infection becomes clinical
        static const double P5; // probability of typhoid death
        //////////JG REMOVE static const double P6; // probability of sterile immunity after acute infection
        static const double P7; // probability of clinical immunity after acute infection
        //////////JG REMOVE static const double P8; // probability of sterile immunity from a subclinical infectin in the clinically immune
        //////////JG REMOVE static const double P9; // probability of sterile immunity from a subclinical infection
        static const double P10; // probability of clinical immunity from a subclinical infection

        static const int _chronic_duration;
        static const int _clinical_immunity_duration;
        //////////JG REMOVE static const int _sterile_immunity_duration;

        // Incubation period by transmission route (taken from Glynn's dose response analysis) assuming low dose for environmental.
        // mean and std dev of log normal distribution
        static const double mpe;
        static const double spe;
        static const double mpf; // math.log(4.7)
        static const double spf;

        // Subclinical infectious duration parameters: mean and standard deviation under and over 30
        static const double mso30;
        static const double sso30;
        static const double msu30;
        static const double ssu30;

        // Acute infectious duration parameters: mean and standard deviation under and over 30
        static const double mao30;
        static const double sao30;
        static const double mau30;
        static const double sau30;

        static const int acute_symptoms_duration; // how long people are symptomatic in days
        static const double CFRU;   // case fatality rate?
        static const double CFRH; // hospitalized case fatality rate?
        static const double treatmentprobability;  // probability of treatment

        // typhoid constants from "OutBase.csv" file
        //////////JG REMOVE static double agechronicmale[200]; //probability of becoming chronic carrier, male
        //////////JG REMOVE static double agechronicfemale[200]; //probability of becoming chronic carrier, female

        // environmental exposure constants
        static const int N50;
        static const double alpha;

    private:
        SusceptibilityTyphoid * typhoid_susceptibility;
        std::map< TransmissionRoute::Enum, float > contagion_population_by_route;

        virtual bool Configure( const Configuration* config );
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
