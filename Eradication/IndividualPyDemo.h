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
#include "InfectionPyDemo.h"
#include "IndividualEnvironmental.h"
#include "SusceptibilityPyDemo.h"

namespace Kernel
{
    class SusceptibilityPyDemo;
    class IIndividualHumanPyDemo : public ISupports
    {
    public:
        virtual bool IsChronicCarrier( bool incidence_only = true ) const = 0;
        virtual bool IsSubClinical( bool incidence_only = true ) const = 0;
        virtual bool IsAcute( bool incidence_only = true ) const = 0;
        virtual bool IsPrePatent( bool incidence_only = true ) const = 0;
    };

    class IndividualHumanPyDemo : public IndividualHuman, public IIndividualHumanPyDemo
    {
        friend class SimulationPyDemo;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(IndividualHumanPyDemo);

    public:
        static IndividualHumanPyDemo *CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual ~IndividualHumanPyDemo(void);

        virtual void CreateSusceptibility(float imm_mod = 1.0, float risk_mod = 1.0);
        virtual void ExposeToInfectivity(float dt = 1.0, const TransmissionGroupMembership_t* transmissionGroupMembership = NULL);

        virtual bool IsChronicCarrier( bool incidence_only = true ) const;
        virtual bool IsSubClinical( bool incidence_only = true ) const;
        virtual bool IsAcute( bool incidence_only = true ) const;
        virtual bool IsPrePatent( bool incidence_only = true ) const;

    protected:

        // New Exposure Pattern
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route );

        IndividualHumanPyDemo(suids::suid id = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual void setupInterventionsContainer();
        virtual void PropagateContextToDependents();

        virtual void UpdateInfectiousness(float dt);
        virtual void Update(float currenttime, float dt);
        virtual Infection* createInfection(suids::suid _suid);
        virtual void AcquireNewInfection(StrainIdentity *infstrain = NULL, int incubation_period_override = -1);
        virtual HumanStateChange GetStateChange() const;

		std::string processPrePatent( float dt );

        // pydemo infection state
        std::string state_to_report; // pydemo status of individual
        std::string last_state_reported; // previous pydemo status of individual
        int chronic_timer;
        int subclinical_timer;
        int acute_timer;
        int prepatent_timer;
        int clinical_immunity_timer;  // timers of days left in state, or UNINIT_TIMER if not used //JG- I'm going to leave clinical immunity in for now. 
        int _subclinical_duration;
        int _prepatent_duration;
        int _acute_duration; // duration of state in days
        bool hasClinicalImmunity; // is immune to clinical infection
        bool isChronic;       // is or will be a chronic carrier (using "Ames" definition)
        int _infection_count;     // number of times infected;
        TransmissionRoute::Enum _routeOfInfection; // how did this person get infected?
        bool isDead;  // is this individual dead?
        bool state_changed;

    private:
        SusceptibilityPyDemo * pydemo_susceptibility;
        std::map< TransmissionRoute::Enum, float > contagion_population_by_route;

        virtual bool Configure( const Configuration* config );
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        ///////////////////////////////////////////////////////////////////////////
        // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, IndividualHumanPyDemo& flags, const unsigned int  file_version );

        ////////////////////////////////////////////////////////////////////////////
#endif
    };
}
