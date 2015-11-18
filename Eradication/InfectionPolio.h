/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "InfectionEnvironmental.h"
#include "PolioDefs.h" // for N_POLIO_SEROTYPES
#include "SusceptibilityPolio.h"

namespace Kernel
{ 
    struct IInfectionPolioReportable : ISupports {
        virtual float GetTotalDuration() const = 0;
        virtual float GetInitialInfectiousness() const = 0;
        virtual float GetInfectiousness() const = 0;
        virtual float GetParalysisTime() const = 0;
        virtual float GetMusocalImmunity() const = 0;
        virtual float GetHumoralImmunity() const = 0;
        virtual int   GetAntigenID() const = 0;
        virtual int   GetGeneticID() const = 0;
    };    
    
    class InfectionPolioConfig : public JsonConfigurable
    {
        friend class IndividualPolio;
        GET_SCHEMA_STATIC_WRAPPER(InfectionPolioConfig)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        /* clorton virtual */ bool Configure( const Configuration* config ) /* clorton override */;

    protected:
        static double         antibody_IRBC_killrate;
        static bool           tracecontact_mode;          // flag for genome mutation algorithm for serial infection tracing
        static int            default_antigen;            // default infection antigenID
        static int            default_genome;             // default infection genomeID
        static float          paralytic_case_mortality;   // (dimensionless) fraction of paralytic cases resulting in death
        static float          evolutionWPVLinearRate;     // (bits / day) mutation rate in the model genome
        static float          evolutionSabinLinearRate[N_POLIO_SEROTYPES];   // (bits / day) maximum mutation rate in model genome, Sabin reversion
        static float          evolutionImmuneRate;        // (bits / day-log2(antibody)) mutation rate factor for log2(neutralizing antibody)
        static float          evolutionHalfmaxReversion;  // (reversion degree) point of reversion at which the reversion rate declines to half maximum rate
        
    };

    class InfectionPolio
        : public InfectionEnvironmental,
          public IInfectionPolioReportable,
          protected InfectionPolioConfig
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        static InfectionPolio *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~InfectionPolio(void);

        virtual void SetParameters(StrainIdentity* infstrain = nullptr, int incubation_period_override = -1) override;
        virtual void InitInfectionImmunology(Susceptibility* _immunity) override;
        virtual void Update(float dt, ISusceptibilityContext* _immunity = nullptr) override;

        // IInfectionPolioReportable methods
        virtual float GetTotalDuration() const override { return total_duration; }
        virtual float GetInitialInfectiousness() const override { return initial_infectiousness; }
        virtual float GetInfectiousness() const override { return infectiousness; }
        virtual float GetParalysisTime() const override { return paralysis_time; }
        virtual float GetMusocalImmunity() const override;
        virtual float GetHumoralImmunity() const override { return cached_humoral_immunity; }
        virtual int   GetAntigenID() const override { return infection_strain->GetAntigenID(); }
        virtual int   GetGeneticID() const override { return infection_strain->GetGeneticID(); }

        void CacheMCWeightOfHost(float ind_mc_weight);

    protected:
        InfectionPolio();
        int   shed_genome_traceContactMode; // genomeID that is shed for contact tracing purposes
        int   immunity_updated; // takes values 0,1
        int   paralysis_reported; // takes values 0,1
        float paralysis_time; // delay from infection to paralysis. individual will be paralyzed if this value is greater than 0
        float initial_infectiousness; // base value of infectiousness based on antibody titer before infection
        float cached_mucosal_immunity; // current mucosal immunity 
        float cached_humoral_immunity; // current humoral immunity
        float host_mc_weight; // (days) stores the MC weight of the individual, use for reporting infections only
        
        float peakFecalLog10VirusTiter; // (TCID50 at the base infectivity pinf of the virus type) the true TCID50 is found by multiplying by relative infectivity for the genome being shed
        float peakOralLog10VirusTiter; //
        float durationFecalInfection; //
        float durationOralInfection; //

        float paralysis_probability; // Probability of paralysis. This was in InitInfectionImmunology, moved to Update, but still only want to calc one time.

        float drug_titer_reduction;
        float drug_infection_duration_reduction;

        bool drug_flag;

        virtual void  evolveStrain(ISusceptibilityPolio* _immunity, float dt);
        virtual float getCurrentTiterFromProfile(float peak_Log10Titer, float infectiousTime, float mu, float sigma); // (TCID50 per volume excretion at time from infection)
        virtual void  setCurrentInfectivity(float relative_infectivity, float infectionTimeFecal, float InfectionTimeOral);// (TCID50 virus per day excreted)

        /* clorton virtual */ const SimulationConfig* params() /* clorton override */;

        InfectionPolio(IIndividualHumanContext *context);
        /* clorton virtual */ void Initialize(suids::suid _suid) /* clorton override */;

        DECLARE_SERIALIZABLE(InfectionPolio);

    /*public:
        virtual ~InfectionPolio(void);

        virtual void InitInfectionImmunology(Susceptibility* _immunity);
        virtual void Update(float dt, Susceptibility* _immunity = nullptr);
    */

        // InfectionPolioReportable methods
        /*virtual const float GetTotalDuration() const { return total_duration; }
        virtual const float GetInitialInfectiousness() const { return initial_infectiousness; }
        virtual const float GetInfectiousness() const { return infectiousness; }
        virtual const float GetParalysisTime() const { return paralysis_time; }
        virtual const float GetMusocalImmunity() const { return cached_mucosal_immunity; }
        virtual const float GetHumoralImmunity() const { return cached_humoral_immunity; }
        virtual const int   GetAntigenID() const { return infection_strain->GetAntigenID(); }
        virtual const int   GetGeneticID() const { return infection_strain->GetGeneticID(); }
*/
    };
}

