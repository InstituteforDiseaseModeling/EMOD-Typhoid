/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "ISerializable.h"
#include "suids.hpp"
#include "Common.h"
#include "Contexts.h"
#include "INodeContext.h"
#include "IInfection.h"

namespace Kernel
{
    struct IMigrate;

    // Interface for controlling objects (e.g. Node)
    struct IIndividualHuman : ISerializable
    {
        // Setup
        virtual void setupMaternalAntibodies(IIndividualHumanContext* mother, INodeContext* node) = 0;
        virtual void AcquireNewInfection(StrainIdentity *infstrain = nullptr, int incubation_period_override = -1) = 0;
        virtual void SetInitialInfections(int init_infs) = 0;
        virtual void SetParameters( INodeContext* pParent, float infsample, float imm_mod, float risk_mod, float mig_mod) = 0;
        virtual void InitializeHuman() = 0;
        virtual void SetMigrationModifier( float modifier ) = 0;

        // Control
        virtual void Update(float current_time, float dt) = 0;
        virtual void UpdateInfectiousness(float dt) = 0;
        virtual void UpdateGroupMembership() = 0;
        virtual void UpdateGroupPopulation(float size_changes) = 0;

        // Inspection
        virtual suids::suid GetSuid() const = 0;
        virtual double GetAge() const = 0;
        virtual bool IsAdult() const = 0;
        virtual int GetGender() const = 0;
        virtual double GetMonteCarloWeight() const = 0;
        virtual bool IsInfected() const = 0;
        virtual bool AtHome() const = 0;
        virtual bool IsOnFamilyTrip() const = 0 ;
        virtual const suids::suid& GetHomeNodeId() const = 0 ;
        virtual bool IsDead() const = 0;

        virtual NewInfectionState::_enum GetNewInfectionState() const = 0;
        virtual HumanStateChange GetStateChange() const = 0;

        virtual IMigrate* GetIMigrate() = 0;
        virtual IIndividualHumanInterventionsContext* GetInterventionsContext() const = 0;

        virtual tProperties* GetProperties() = 0;
        virtual const std::string& GetPropertyReportString() const = 0;
        virtual void SetPropertyReportString( const std::string& str ) = 0;

        virtual INodeContext* GetParent() const = 0;

        virtual IIndividualHumanEventContext *GetEventContext() = 0;    // access to specific attributes of the individual useful for events
        virtual float GetAcquisitionImmunity() const = 0;               // KM: For downsampling based on immune status.  For now, just takes perfect immunity; can be updated to include a threshold.  Unclear how to work with multiple strains or waning immunity.
        virtual bool IsPossibleMother() const = 0;
        virtual void UpdateMCSamplingRate(float current_sampling_rate) = 0;
        virtual bool IsPregnant() const = 0;
        virtual bool UpdatePregnancy(float dt = 1) = 0; // returns true if birth happens this time step and resets is_pregnant to false
        virtual void InitiatePregnancy(float duration = (DAYSPERWEEK * WEEKS_FOR_GESTATION)) = 0;
        virtual float GetInfectiousness() const = 0;
        virtual ProbabilityNumber getProbMaternalTransmission() const = 0;
        virtual ISusceptibilityContext* GetSusceptibilityContext() const = 0;
        virtual inline Kernel::suids::suid GetParentSuid() const = 0;
        virtual bool IsMigrating() = 0;
        virtual void ClearNewInfectionState() = 0;
        virtual const infection_list_t& GetInfections() const = 0;
        virtual int GetAbovePoverty() const = 0;    // financially secure = 1, less financially secure = 0
        virtual float GetImmunityReducedAcquire() const = 0;
        virtual float GetInterventionReducedAcquire() const = 0;
        virtual const suids::suid& GetMigrationDestination() = 0;

        // Migration
        virtual void SetContextTo( INodeContext* ) = 0;
        virtual void SetGoingOnFamilyTrip( suids::suid migrationDestination, 
                                           MigrationType::Enum migrationType, 
                                           float timeUntilTrip, 
                                           float timeAtDestination,
                                           bool isDestinationNewHome ) = 0;
        virtual void SetWaitingToGoOnFamilyTrip() = 0;
        virtual void GoHome() = 0;

        virtual ~IIndividualHuman() {}
    };

    struct IIndividualHumanEventContext;
    struct IIndividualHumanInterventionsContext;

    // Interface for contained objects (e.g. Susceptibility, Infection)
// clorton    struct IIndividualHumanContext : ISupports
// clorton    {
// clorton        virtual suids::suid GetSuid() const = 0;
// clorton
// clorton        virtual suids::suid GetNextInfectionSuid() = 0;
// clorton        virtual ::RANDOMBASE* GetRng() = 0; 
// clorton
// clorton        virtual IIndividualHumanInterventionsContext *GetInterventionsContext() const = 0; // internal components of individuals interact with interventions via this interface
// clorton        
// clorton        virtual IIndividualHumanInterventionsContext *GetInterventionsContextbyInfection(Infection* infection) = 0; // internal components of individuals interact with interventions via this interface
// clorton        virtual IIndividualHumanEventContext *GetEventContext() = 0;                       // access to specific attributes of the individual useful for events
// clorton        virtual ISusceptibilityContext *GetSusceptibilityContext() const = 0;              // access to immune attributes useful for infection, interventions, reporting, etc.
// clorton
// clorton        virtual const NodeDemographics* GetDemographics() const = 0;
// clorton        virtual const NodeDemographicsDistribution* GetDemographicsDistribution(std::string) const = 0;
// clorton        
// clorton        virtual void UpdateGroupMembership() = 0;
// clorton        virtual void UpdateGroupPopulation(float size_changes) = 0;
// clorton    };


/* In IndividualEventContext.h
    // Interface for interventions, etc.
    struct IIndividualHumanEventContext : ISupports
    {

    };
*/

/* In Interventions.h
    struct IIndividualHumanInterventionsContext : ISupports
    {

    };
*/
}
