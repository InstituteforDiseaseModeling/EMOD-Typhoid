/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "ISupports.h"
#include "EnumSupport.h"

namespace Kernel
{
    ENUM_DEFINE(PolioVaccineType,
        ENUM_VALUE_SPEC(tOPV   , 0)
        ENUM_VALUE_SPEC(bOPV   , 1)
        ENUM_VALUE_SPEC(mOPV_1 , 2)
        ENUM_VALUE_SPEC(mOPV_2 , 3)
        ENUM_VALUE_SPEC(mOPV_3 , 4)
        ENUM_VALUE_SPEC(eIPV   , 5))

    class StrainIdentity;

    struct INodePolio : ISupports
    {
        virtual float GetMeanAgeInfection() const = 0;
        virtual float GetNewDiseaseSusceptibleInfections() const = 0;
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

    struct IInfectionPolioReportable : ISupports {
        virtual float GetTotalDuration() const = 0;
        virtual float GetInitialInfectiousness() const = 0;
        virtual float GetInfectiousness() const = 0;
        virtual float GetParalysisTime() const = 0;
        virtual float GetMucosalImmunity() const = 0;
        virtual float GetHumoralImmunity() const = 0;
        virtual int   GetAntigenID() const = 0;
        virtual int   GetGeneticID() const = 0;
    };    
    
    typedef std::list<const IInfectionPolioReportable*> infection_polio_reportable_list_t;

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



    class IIndividualHumanPolio : public ISupports
    {
    public:
        virtual ISusceptibilityPolioReportable *GetSusceptibilityReporting() const = 0;
        virtual int GetParalysisVirusTypeMask() const = 0;
        virtual float GetAgeOfMostRecentInfection(void) = 0;
        virtual bool GetSusceptibleStatus(int pvType) = 0;
        virtual void ClearAllNewInfectionByStrain(void) = 0;
        virtual const infection_polio_reportable_list_t *GetNewInfectionReportables() const = 0;
        virtual const infection_polio_reportable_list_t *GetInfectionReportables(bool getNewInfectionsOnly) const = 0;
    };

    struct IPolioVaccine : ISupports
    {
        virtual PolioVaccineType::Enum   GetVaccineType()            const = 0;
    };

    struct IPolioVaccineEffects : ISupports
    {
        virtual std::list<IPolioVaccine*>& GetNewVaccines() = 0;
        virtual void ClearNewVaccines() = 0;
        virtual ~IPolioVaccineEffects() { }
    };

    struct IPolioDrugEffects : ISupports
    {
        virtual float get_titer_efficacy() const = 0;
        virtual float get_infection_duration_efficacy() const = 0;

        virtual ~IPolioDrugEffects() { }
    };

    // this container becomes a help implementation member of the relevant IndividualHuman class 
    // it needs to implement consumer interfaces for all the relevant intervention types
    struct IPolioDrugEffectsApply : public ISupports
    {
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ) = 0;
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ) = 0;
        virtual void ApplyDrugTiterEffect( float rate ) = 0;
        virtual void ApplyDrugDurationEffect( float rate ) = 0;
    };

}
