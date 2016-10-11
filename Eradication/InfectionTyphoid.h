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
#include "InfectionEnvironmental.h"
#include "TyphoidDefs.h" // for N_TYPHOID_SEROTYPES

namespace Kernel
{
    class InfectionTyphoidConfig : public JsonConfigurable
    {
        friend class IndividualTyphoid;
        GET_SCHEMA_STATIC_WRAPPER(InfectionTyphoidConfig)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        bool Configure( const Configuration* config );

    protected:
       
    };

    class IInfectionTyphoid : public ISupports
    {
        public:
        virtual void Clear() = 0;
    };

    class InfectionTyphoid
        : public InfectionEnvironmental
        , public IInfectionTyphoid 
        , protected InfectionTyphoidConfig
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        static InfectionTyphoid *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~InfectionTyphoid(void);

        virtual void SetParameters(StrainIdentity* infstrain = NULL, int incubation_period_override = -1) override;
        virtual void InitInfectionImmunology(ISusceptibilityContext* _immunity) override;
        virtual void Update(float dt, ISusceptibilityContext* _immunity = NULL) override;
        void SetMCWeightOfHost(float ind_mc_weight);
        virtual void Clear() override;
        virtual float GetInfectiousness() const override;

        // InfectionTyphoidReportable methods
    protected:
        InfectionTyphoid(); 

        const SimulationConfig* params();

        InfectionTyphoid(IIndividualHumanContext *context);
        void Initialize(suids::suid _suid);
        void handlePrepatentExpiry();
        void handleAcuteExpiry();
        void handleSubclinicalExpiry();
		int treatment_multiplier;
        int chronic_timer;
        int subclinical_timer;
        int acute_timer;
        int prepatent_timer;
        int clinical_immunity_timer;  // timers of days left in state, or UNINIT_TIMER if not used //JG- I'm going to leave clinical immunity in for now. 
        int _subclinical_duration;
        int _prepatent_duration;
        int _acute_duration; // duration of state in days
        bool isDead;  // is this individual dead?
        std::string last_state_reported; // previous typhoid status of individual
        static const int _chronic_duration;
        static const int _clinical_immunity_duration;

        static const int acute_treatment_day; // what day individuals are treated
        static const float CFRU;   // case fatality rate?
        static const float CFRH; // hospitalized case fatality rate?
        static const float treatmentprobability;  // probability of treatment
	

        // Incubation period by transmission route (taken from Glynn's dose response analysis) assuming low dose for environmental.
        // mean and std dev of log normal distribution
        static const float mph;
        static const float sph;
        static const float mpm;
        static const float spm;
		static const float mpl;
        static const float spl;

        // Subclinical infectious duration parameters: mean and standard deviation under and over 30
        static const float mso30;
        static const float sso30;
        static const float msu30;
        static const float ssu30;

        // Acute infectious duration parameters: mean and standard deviation under and over 30
        static const float mao30;
        static const float sao30;
        static const float mau30;
        static const float sau30;

        static const float P10; // probability of clinical immunity from a subclinical infection

    private:
        DECLARE_SERIALIZABLE(InfectionTyphoid);
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;

        template<class Archive>
        friend void serialize(Archive & ar, InfectionTyphoid& inf, const unsigned int file_version );
#endif
    };
}

