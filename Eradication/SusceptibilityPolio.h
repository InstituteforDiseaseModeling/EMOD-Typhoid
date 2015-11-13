/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "SusceptibilityEnvironmental.h"
#include "StrainIdentity.h"
#include "PolioDefs.h"

#ifndef N_MAX_POLIO_GENOMES
#define N_MAX_POLIO_GENOMES 1048576
#endif

namespace Kernel
{
    class SusceptibilityPolioConfig: public JsonConfigurable
    {
        GET_SCHEMA_STATIC_WRAPPER(SusceptibilityPolioConfig)

        friend class IndividualHumanPolio;

    public:
        virtual bool Configure( const Configuration* config ) override;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
        static float x_population_immunity;
        static float mucosalImmIPV;
        static float mucosalImmIPVOPVExposed;
        static float TauNAb;
        static float paralytic_immunity_titer;
        static float waning_humoral_rate_fast;
        static float waning_humoral_rate_slow;
        static float waning_humoral_fast_fraction;
        static float waning_mucosal_rate_fast;
        static float waning_mucosal_rate_slow;
        static float waning_mucosal_fast_fraction;
        static float acquire_rate_default_polio;
        static float maternal_log2NAb_mean;
        static float maternal_log2NAb_std;
        static float maternalAbHalfLife;
        static float excrement_load;
    };

    class ISusceptibilityPolio : public ISupports
    {
    public:
        virtual void SetNewInfectionByStrain(StrainIdentity* infection_strain) = 0;
        //immunity
        virtual float GetFecalInfectiousDuration(StrainIdentity* strain_id) = 0; // (days)
        virtual float GetOralInfectiousDuration(StrainIdentity* strain_id) = 0; // days
        virtual float GetPeakFecalLog10VirusTiter(StrainIdentity* strain_id) = 0; // (TCID50 virus per day excreted)
        virtual float GetPeakOralLog10VirusTiter(StrainIdentity* strain_id) = 0; // (TCID50 virus per day excreted)
        virtual float GetParalysisTime(StrainIdentity* strain_id, float infect_duration) = 0; // days incubation to paralysis. returns -1 if no paralysis
        virtual float GetReversionDegree(StrainIdentity* strain_id) = 0; // (on scale from 0 to 1)
        virtual void DecrementInfection(StrainIdentity* infection_strain) = 0;
        virtual void IncrementInfection(StrainIdentity* infection_strain) = 0;
        virtual void ApplyImmumeResponseToInfection(StrainIdentity* infection_strain) = 0;
        virtual bool GetSusceptibleStatus(int pvType) = 0;
        virtual float GetRandNBounded(void) = 0;
        virtual int   GetSerotype(StrainIdentity* strain_id) = 0;
        virtual float GetMucosalImmunity(StrainIdentity* strain_id) = 0; // (reciprocal titer linear units)
        virtual float GetHumoralImmunity(StrainIdentity* strain_id) = 0; // (reciprocal titer linear units)
        virtual void  ResetTimeSinceLastInfection(int serotype) = 0;
   };

    class ISusceptibilityPolioReportable : public ISupports
    {
        public:
        virtual void GetSheddingTiter(float sheddingTiters[])               const = 0;
        virtual void GetHumoralNAb(float humoralNAb[])                      const = 0;
        virtual void GetMucosalNAb(float mucosalNAb[])                      const = 0;
        virtual void GetMaternalSerumNAb(float maternalSerumNAb[])          const = 0;
        virtual void GetHumoralMemoryNAb(float humoralMemoryNAb[])          const = 0;
        virtual void GetMucosalMemoryNAb(float mucosalMemoryNAb[])          const = 0;
        virtual void GetTimeSinceLastInfection(float times[])               const = 0;
        virtual void GetVaccineDosesReceived(int vaccineDosesReceived[])    const = 0;
        virtual const int* GetInfectionStrains()                            const = 0;
        virtual const int* GetNewInfectionsByStrain()                       const = 0;
        virtual float GetIndividualAcquireRisk()                            const = 0;
        virtual bool IsSeropositive( unsigned char serotype)                const = 0;
    };

    class SusceptibilityPolio :
        public SusceptibilityEnvironmental,
        public SusceptibilityPolioConfig,
        public ISusceptibilityPolio,
        public ISusceptibilityPolioReportable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()

    protected:
        float getProbabilityInfectionSingleStrain(StrainIdentity* strain_id, float challenge_dose);
        void  challengeIPV(float* dose_Dantigen_content);
        bool  isOPVExposed(int serotype);
        void  updateMemoryImmunity(void);
        virtual void validateStrainSettings(void);

        float shedding_titer[N_POLIO_VIRUS_TYPES]; // (TCID50/g feces) amount of each virus by type currently being shed
        float humoralNAb[N_POLIO_SEROTYPES];       // (reciprocal titer, linear units)
        float mucosalNAb[N_POLIO_SEROTYPES];       // (reciprocal titer, linear units)
        float maternalSerumNAb[N_POLIO_SEROTYPES]; // (reciprocal titer, linear units)
        float humoralMemoryNAb[N_POLIO_SEROTYPES]; // (reciprocal titer, linear units)
        float mucosalMemoryNAb[N_POLIO_SEROTYPES]; // (reciprocal titer, linear units)

        float humoral_fastDecayCompartment[N_POLIO_SEROTYPES]; // (reciprocal titer, linear units) antibody remaining with a fast decay rate
        float mucosal_fastDecayCompartment[N_POLIO_SEROTYPES]; // (reciprocal titer, linear units) antibody remaining with a fast decay rate

        float time_since_last_infection[N_POLIO_SEROTYPES]; // (days) time elapsed from exposure
        float time_since_last_IPV[N_POLIO_SEROTYPES]; // (days) time elapsed from exposure
        float individual_acquire_risk; // (dimensionless) demographic- and age-dependent acquisition risk, multiplies with contact_acquire_polio to give the individual's contact acquisition
        int   vaccine_doses_received[N_POLIO_VACCINES];     // {tOPV bOPV mOPV1 mOPV2 mOPV3 IPV} counters for number of doses already received
        int   vaccine_doses_received_by_type[N_POLIO_VIRUS_TYPES]; // number of doses received by type {1,2,3}

        int infectionStrains[N_POLIO_VIRUS_TYPES];          // number of current infections of each virus type
        int newInfectionByStrain[N_POLIO_VIRUS_TYPES];      // number of new infections of each virus type

        static float ReferenceEIPV_Dantigen[N_POLIO_SEROTYPES];

        // old data:
        // multiplicative factor for antibody titer (not in log units) following challenge, analysis in "immune_response_OPV.xls"
        // from Glezen1966, Faden1990, Ghendon1961

        SusceptibilityPolio();
        SusceptibilityPolio(IIndividualHumanContext *context);
        /* clorton virtual */ void Initialize(float age, float immmod, float riskmod) /* clorton override */;
        void AddVaccineToInterventionsContainer(int type, float time_since_vaccination);

        DECLARE_SERIALIZABLE(SusceptibilityPolio);

    public:
        static SusceptibilityPolio *CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod);
        virtual ~SusceptibilityPolio(void);

        virtual void Update(float dt = 0.0) override;
        virtual void UpdateInfectionCleared() override;
        virtual void GetNewInterventionsForIndividual(float dt, int* n_vaccine_strains, StrainIdentity* strain_id, float* dose_titer);
        virtual void GetProbabilityInfected(StrainIdentity* strain_id, float *acquired_virus, int n_challenge_strains, float* probability_infected);
        virtual float GetFecalInfectiousDuration(StrainIdentity* strain_id) override; // (days)
        virtual float GetOralInfectiousDuration(StrainIdentity* strain_id) override; // days
        virtual float GetPeakFecalLog10VirusTiter(StrainIdentity* strain_id) override; // (TCID50 virus per day excreted)
        virtual float GetPeakOralLog10VirusTiter(StrainIdentity* strain_id) override; // (TCID50 virus per day excreted)
        virtual float GetParalysisTime(StrainIdentity* strain_id, float infect_duration) override; // days incubation to paralysis. returns -1 if no paralysis
        virtual float GetReversionDegree(StrainIdentity* strain_id) override; // (on scale from 0 to 1)
        virtual void ApplyImmumeResponseToInfection(StrainIdentity* infection_strain) override;
        virtual void DecrementInfection(StrainIdentity* infection_strain) override;
        virtual void IncrementInfection(StrainIdentity* infection_strain) override;
        virtual const int* GetInfectionStrains() const override;
        virtual void AccumulateNewInfectionCount(float ind_MCweight, float* inf_count);
        void ClearAllNewInfectionByStrain(void);
        virtual void SetNewInfectionByStrain(StrainIdentity* infection_strain) override;
        virtual bool GetSusceptibleStatus(int pvType) override;
        virtual float GetRandNBounded(void) override;
        virtual void waneAntibodyTitersFromLastEvent(void);

        virtual int   GetSerotype(StrainIdentity* strain_id) override;
        virtual float GetMucosalImmunity(StrainIdentity* strain_id) override; // (reciprocal titer linear units)
        virtual float GetHumoralImmunity(StrainIdentity* strain_id) override; // (reciprocal titer linear units)
        float GetDefaultAcquireRate(void);
        virtual void  ResetTimeSinceLastInfection(int serotype) override;
        virtual void  ResetTimeSinceLastIPV(int serotype);

        // ISusceptibilityPolioReporting
        virtual void GetSheddingTiter(float sheddingTiters[])            const override;
        virtual void GetHumoralNAb(float humoralNAb[])                   const override;
        virtual void GetMucosalNAb(float mucosalNAb[])                   const override;
        virtual void GetMaternalSerumNAb(float maternalSerumNAb[])       const override;
        virtual void GetHumoralMemoryNAb(float humoralMemoryNAb[])       const override;
        virtual void GetMucosalMemoryNAb(float mucosalMemoryNAb[])       const override;
        virtual void GetTimeSinceLastInfection(float times[])            const override;
        virtual void GetTimeSinceLastIPV(float times[])                  const;
        virtual void GetVaccineDosesReceived(int vaccineDosesReceived[]) const override;
        virtual const int* GetNewInfectionsByStrain()                    const override;
        virtual float GetIndividualAcquireRisk()                         const override;
        virtual bool IsSeropositive( unsigned char serotype)             const override;

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, SusceptibilityPolio&, const unsigned int  file_version );
#endif
    };
}

