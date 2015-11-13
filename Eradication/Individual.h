/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <list>
#include <map>
#include <string>

#include "BoostLibWrapper.h"
#include "Common.h"
#include "Configure.h"
#include "Contexts.h"
#include "IInfectable.h"
#include "IMigrate.h"
#include "IndividualEventContext.h"
#include "InterventionsContainer.h" // for serialization code to compile
#include "TransmissionGroupMembership.h"
#include "SimulationEnums.h"
#include "suids.hpp"
#include "IContagionPopulation.h"
#include "IIndividualHuman.h"
#include "Types.h" // for ProbabilityNumber
#include "INodeContext.h"

class Configuration;
class RANDOMBASE;

namespace Kernel
{
    struct IIndividualHumanInterventionsContext;
    class  Infection;
    class  InterventionsContainer;
    struct ISusceptibilityContext;
    class  NodeNetwork;
    class  StrainIdentity;
    class  Susceptibility;

    class IndividualHumanConfig : public JsonConfigurable
    {
        GET_SCHEMA_STATIC_WRAPPER(IndividualHumanConfig)
        friend class Simulation;

    protected:
        static bool aging;

        // migration parameters
        static int roundtrip_waypoints;
        static MigrationPattern::Enum migration_pattern;
        static float local_roundtrip_prob;
        static float air_roundtrip_prob;
        static float region_roundtrip_prob;
        static float sea_roundtrip_prob;

        // duration rates
        static float local_roundtrip_duration_rate;
        static float air_roundtrip_duration_rate;
        static float region_roundtrip_duration_rate;
        static float sea_roundtrip_duration_rate;

        static int infection_updates_per_tstep;
        static bool immunity;
        static int max_ind_inf;
        static bool superinfection;
        static float x_othermortality;

        virtual bool Configure( const Configuration* config ) override;

        void RegisterRandomWalkDiffusionParameters();
        void RegisterSingleRoundTripsParameters();
        void RegisterWaypointsHomeParameters();

        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    };

    class IndividualHuman : public IIndividualHuman,
                            public IIndividualHumanContext,
                            public IIndividualHumanEventContext,
                            public IInfectable,
                            public IInfectionAcquirable,
                            public IMigrate,
                            protected IndividualHumanConfig
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:

        static IndividualHuman *CreateHuman();
        static IndividualHuman *CreateHuman(INodeContext *context, suids::suid id, float MCweight = 1.0f, float init_age = 0.0f, int gender = 0, float init_poverty = 0.5f);
        virtual void InitializeHuman();
        virtual ~IndividualHuman();

        virtual void Update(float currenttime, float dt) override;

        // IIndividualHumanContext
        virtual suids::suid GetSuid() const override;
        virtual suids::suid GetNextInfectionSuid() override;
        virtual ::RANDOMBASE* GetRng() override;

        virtual IIndividualHumanInterventionsContext* GetInterventionsContext() const override;
        virtual IIndividualHumanInterventionsContext* GetInterventionsContextbyInfection(Infection* infection) override;
        virtual IIndividualHumanEventContext*         GetEventContext() override;
        virtual ISusceptibilityContext*               GetSusceptibilityContext() const override;

        virtual const Kernel::NodeDemographics*     GetDemographics() const override;
        virtual const NodeDemographicsDistribution* GetDemographicsDistribution(std::string key) const override;

        // IIndividualHumanEventContext methods
        inline  bool   IsPregnant()           const { return is_pregnant; };
        inline  int    GetAbovePoverty()      const { return above_poverty; } // financially secure = 1, less financially secure = 0
        inline  double GetAge()               const { return m_age; }
        inline  int    GetGender()            const { return m_gender; }
        inline  double GetMonteCarloWeight()  const { return m_mc_weight; }
        virtual bool   IsPossibleMother()     const;
        inline  bool   IsInfected()           const { return m_is_infected; }
        virtual float  GetAcquisitionImmunity()          const;  // KM: For downsampling based on immune status.  For now, just takes perfect immunity; can be updated to include a threshold.  Unclear how to work with multiple strains or waning immunity.
        inline HumanStateChange GetStateChange() const { return StateChange; }
        virtual void Die( HumanStateChange ) override;
        virtual INodeEventContext   * GetNodeEventContext() override; // for campaign cost reporting in e.g. HealthSeekingBehavior
        virtual tProperties* GetProperties() override;

        // Migration
        virtual void ImmigrateTo(Node* destination_node) override;
        virtual const suids::suid& GetMigrationDestination() override;
        void SetMigrationDestination(suids::suid destination);
        bool IsMigrating();
        virtual void CheckForMigration(float currenttime, float dt);
        void SetNextMigration();

        // Heterogeneous intra-node transmission
        virtual void UpdateGroupMembership() override;
        virtual void UpdateGroupPopulation(float size_changes) override;

        // Initialization
        virtual void SetInitialInfections(int init_infs);
        virtual void SetParameters(float infsample, float imm_mod, float risk_mod, float mig_mod); // specify each parameter, default version of SetParams()
        virtual void CreateSusceptibility(float imm_mod=1.0, float risk_mod=1.0);
        virtual void setupMaternalAntibodies(IIndividualHumanContext* mother, INodeContext* node);

        // Infections
        virtual void ExposeToInfectivity(float dt, const TransmissionGroupMembership_t* transmissionGroupMembership);
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route = TransmissionRoute::TRANSMISSIONROUTE_ALL ) override;
        virtual void AcquireNewInfection(StrainIdentity *infstrain = nullptr, int incubation_period_override = -1) override;
        virtual const infection_list_t &GetInfections() const override;
        virtual void UpdateInfectiousness(float dt);
        virtual bool InfectionExistsForThisStrain(StrainIdentity* check_strain_id);
        virtual void ClearNewInfectionState();
        inline NewInfectionState::_enum GetNewInfectionState() const { return m_new_infection_state; }
        virtual inline float GetInfectiousness() const { return infectiousness; }

        virtual float GetImmunityReducedAcquire() const;
        virtual float GetInterventionReducedAcquire() const override;

        // Births and deaths
        virtual bool  UpdatePregnancy(float dt=1); // returns true if birth happens this time step and resets is_pregnant to false
        void InitiatePregnancy(float duration = (DAYSPERWEEK * WEEKS_FOR_GESTATION));
        virtual void CheckVitalDynamics(float currenttime, float dt=1.0); // non-disease mortality
        // update and set dynamic MC weight
        void UpdateMCSamplingRate(float current_sampling_rate);

        // Assorted getters and setters
        virtual void SetContextTo(INodeContext* context);
        virtual INodeContext* GetParent() const override;
        inline Kernel::suids::suid GetParentSuid() const { return parent->GetSuid(); }
        virtual ProbabilityNumber getProbMaternalTransmission() const;

    protected:

        // Core properties
        suids::suid suid;
        float m_age;
        int   m_gender;
        float m_mc_weight;
        float m_daily_mortality_rate;
        int   above_poverty;     // financially secure = 1, less financially secure = 0
        bool  is_pregnant;      // pregnancy variables for vital_birth_dependence==INDIVIDUAL_PREGNANCIES
        float pregnancy_timer;

        // Immune system, infection(s), intervention(s), and transmission properties
        Susceptibility*               susceptibility;   // individual susceptibility (i.e. immune system)
        infection_list_t              infections;
        InterventionsContainer*       interventions;
        TransmissionGroupMembership_t transmissionGroupMembership;
        map<string, TransmissionGroupMembership_t> transmissionGroupMembershipByRoute;

        // Infections
        bool  m_is_infected;    // TODO: replace with more sophisticated strain-tracking capability
        float infectiousness;   // infectiousness calculated over all Infections and passed back to Node
        float Inf_Sample_Rate;  // EAW: unused currently
        int   cumulativeInfs;   // counter of total infections over individual's history

        NewInfectionState::_enum  m_new_infection_state; // to flag various types of infection state changes
        HumanStateChange          StateChange;           // to flag that the individual has migrated or died

        // Migration
        float migration_mod;
        int   migration_type;
        suids::suid  migration_destination;
        float time_to_next_migration; // JPS: do we want to store this as an absolute time instead of offset?
        bool  will_return;
        bool  outbound;
        int   max_waypoints;    // maximum waypoints a trip can have before returning home
        std::vector<suids::suid> waypoints;
        std::vector<int>         waypoints_trip_type;

        tProperties Properties;

        INodeContext* parent;   // Access back to node/simulation methods

        IndividualHuman(INodeContext *context);
        IndividualHuman(suids::suid id = suids::nil_suid(), float MCweight = 1.0f, float init_age = 0.0f, int gender = 0, float init_poverty = 0.5f);

        virtual Infection* createInfection(suids::suid _suid); // factory method (overridden in derived classes)
        virtual void setupInterventionsContainer();            // derived classes can customize the container, and hence the interventions supported, by overriding this method
        virtual void applyNewInterventionEffects(float dt);    // overriden when interventions (e.g. polio vaccine) updates individual properties (e.g. immunity)

        // Infection updating
        virtual bool SetNewInfectionState(InfectionStateChange::_enum inf_state_change);
        virtual void ReportInfectionState();

        bool AtHome() const;

        virtual void PropagateContextToDependents();
        INodeTriggeredInterventionConsumer* broadcaster;

        virtual IIndividualHumanContext* GetContextPointer();

        DECLARE_SERIALIZABLE(IndividualHuman);

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI

        // From http://www.boost.org/doc/libs/1_59_0/libs/serialization/doc/tutorial.html
        friend class boost::serialization::access;
        template<class Archive>
        void save(Archive& ar, const unsigned int version) const;
        template<class Archive>
        void load(Archive& ar, const unsigned int version);

        template<class Archive>
        friend void serialize(Archive& ar, IndividualHuman& human, const unsigned int version);

        std::string toJson();
        void fromJson(std::string& json);
#endif
    };
}
