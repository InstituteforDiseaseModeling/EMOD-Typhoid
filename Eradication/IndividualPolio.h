/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Contexts.h"
#include "InfectionPolio.h"
#include "IndividualEnvironmental.h"
#include "SusceptibilityPolio.h"

namespace Kernel
{
    typedef std::list<const IInfectionPolioReportable*> infection_polio_reportable_list_t;

    class SusceptibilityPolio;
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

    class IndividualHumanPolio : public IndividualHumanEnvironmental, public IIndividualHumanPolio
    {
        friend class SimulationPolio;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE()
        GET_SCHEMA_STATIC_WRAPPER(IndividualHumanPolio);

    public:
        static IndividualHumanPolio *CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual ~IndividualHumanPolio(void) {}

        virtual void CreateSusceptibility(float imm_mod = 1.0, float risk_mod = 1.0) override;
        virtual void ExposeToInfectivity(float dt = 1.0, const TransmissionGroupMembership_t* transmissionGroupMembership = nullptr) override;
        virtual ISusceptibilityPolioReportable *GetSusceptibilityReporting() const override;
        virtual void ClearNewInfectionState() override;

        virtual void ClearAllNewInfectionByStrain(void) override;
        virtual int CountInfections(void) const;
        virtual float GetAgeOfMostRecentInfection(void) override;
        virtual bool GetSusceptibleStatus(int pvType) override;
        virtual int GetParalysisVirusTypeMask() const override;

        virtual const infection_polio_reportable_list_t *GetNewInfectionReportables() const override;
        virtual const infection_polio_reportable_list_t *GetInfectionReportables(bool getNewInfectionsOnly=true) const override;

    protected:

        // POLIO, just temporarily here
        static bool ReportSabinWildPhenotypeAsWild; // changes categorization from VDPV to WPV for reporting based on phenotype

        float age_most_recent_infection;
        int paralysisVirusTypeMask; // Bit mask with bit 1=WPV1, 2=WPV2, 3=WPV3, 4=VDPV1, 5=VDPV2, 6=VDPV3

        virtual void applyNewInterventionEffects(float dt) override;

        // Factory methods
        virtual Infection* createInfection(suids::suid _suid) override;

        // New Exposure Pattern
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route ) override;

        IndividualHumanPolio(suids::suid id = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual bool SetNewInfectionState(InfectionStateChange::_enum inf_state_change) override;
        virtual void ReportInfectionState() override;

        virtual void setupInterventionsContainer() override;
        virtual void PropagateContextToDependents() override;

        DECLARE_SERIALIZABLE(IndividualHumanPolio);

    private:
        SusceptibilityPolio * polio_susceptibility;

        virtual bool Configure( const Configuration* config ) override;

        // IIndividualHumanPolio

        //virtual const infection_polio_reportable_list_t *GetNewInfectionReportables() const;

/*    private:
        SusceptibilityPolio * polio_susceptibility;
*/
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        ///////////////////////////////////////////////////////////////////////////
        // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, IndividualHumanPolio& flags, const unsigned int  file_version );

        ////////////////////////////////////////////////////////////////////////////
#endif
    };
}
