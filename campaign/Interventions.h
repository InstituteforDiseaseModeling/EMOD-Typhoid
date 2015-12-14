/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>

#include "IdmApi.h"

#include "ISupports.h"
#include "Configure.h"                 // for JsonConfigurable
#include "InterventionEnums.h"         // for IndividualEventTriggerType enum
#include "VectorEnums.h"               // for larval habitat targets
#include "InterventionValidator.h"
#ifdef ENABLE_TB
#include "TBDrugTypeParameters.h" //Copying the malariadrugtypeparameters
#endif

#include "ISerializable.h"
#include "IArchive.h"

// Could make this an inline function (for folks who hate macros), or even RANDOM
// method, but this seems like the right solution, at least for now. This really
// is just a utility shortcut for interventions at this point.
#define SMART_DRAW(x) \
        ( x > 0 && ( x == 1.0 || parent->GetRng()->e() < x ) )

namespace Kernel
{
    struct IIndividualHumanContext;
    struct IIndividualHumanEventContext;

    struct IDMAPI ICampaignCostObserver : ISupports
    {
        virtual void notifyCampaignExpenseIncurred( float expenseIncurred, const IIndividualHumanEventContext * pIndiv ) = 0;
        virtual void notifyCampaignEventOccurred( /*const*/ ISupports * pDistributedIntervention, /*const*/ ISupports * pDistributor, /*const*/ IIndividualHumanContext * pDistributeeIndividual ) = 0;
        virtual ~ICampaignCostObserver() {}
    };

    struct IIndividualHumanInterventionsContext;

#pragma warning(push)
#pragma warning(disable: 4251)
    struct IDMAPI IDistributableIntervention : ISerializable
    {
        // Distribute transfers ownership of this object to the context if it succeeds, the context becomes responsible for freeing it
        // returns false if cannot distribute to the individual represented by this context, for whatever reason
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pICCO ) = 0;
        virtual void SetContextTo(IIndividualHumanContext *context) = 0;
        virtual void Update(float dt) = 0;
        virtual bool Expired() = 0;
        virtual void ValidateSimType( const std::string& simTypeStr ) = 0;
        virtual IDistributableIntervention * Clone()  = 0;

        virtual ~IDistributableIntervention() { }
    };
#pragma warning(pop)

    struct IIndividualHumanInterventionsContext : ISerializable // ISupports
    {
        // specific helpers for interoperating with the interventions container might go here
        virtual void SetContextTo(IIndividualHumanContext *context) = 0;
        virtual IIndividualHumanContext* GetParent() = 0;
        virtual std::list<IDistributableIntervention*> GetInterventionsByType(const std::string &type_name) = 0;
        virtual void PurgeExisting( const std::string &iv_name ) = 0;

        virtual ~IIndividualHumanInterventionsContext() {}
    };

    struct INodeEventContext;
    struct IEventCoordinator2;

    struct IDMAPI INodeDistributableIntervention : ISupports
    {
        // Distribute transfers ownership of this object to the context if it succeeds, the context becomes responsible for freeing it
        // returns false if cannot distribute to the individual represented by this context, for whatever reason
        virtual bool Distribute(INodeEventContext *context, IEventCoordinator2* pEC = nullptr ) = 0;
        virtual void SetContextTo(INodeEventContext *context) = 0;
        virtual void Update(float dt) = 0;
        virtual void ValidateSimType( const std::string& simTypeStr ) = 0;

        virtual ~INodeDistributableIntervention() { }
    };

    struct IVectorInterventionsEffects : ISupports
    {
        virtual float GetDieBeforeFeeding() = 0;
        virtual float GetHostNotAvailable() = 0;
        virtual float GetDieDuringFeeding() = 0;
        virtual float GetDiePostFeeding() = 0;
        virtual float GetSuccessfulFeedHuman() = 0;
        virtual float GetSuccessfulFeedAD() = 0;
        virtual float GetOutdoorDieBeforeFeeding() = 0;
        virtual float GetOutdoorHostNotAvailable() = 0;
        virtual float GetOutdoorDieDuringFeeding() = 0;
        virtual float GetOutdoorDiePostFeeding() = 0;
        virtual float GetOutdoorSuccessfulFeedHuman() = 0;
        virtual float GetblockIndoorVectorAcquire() = 0;
        virtual float GetblockIndoorVectorTransmit() = 0;
        virtual float GetblockOutdoorVectorAcquire() = 0;
        virtual float GetblockOutdoorVectorTransmit() = 0;
        virtual ~IVectorInterventionsEffects() { }
    };

    struct INodeVectorInterventionEffects : ISupports
    {
        virtual float GetLarvalKilling(VectorHabitatType::Enum) = 0;
        virtual float GetLarvalHabitatReduction(VectorHabitatType::Enum) = 0;
        virtual float GetVillageSpatialRepellent() = 0;
        virtual float GetADIVAttraction() = 0;
        virtual float GetADOVAttraction() = 0;
        virtual float GetPFVKill() = 0;
        virtual float GetOutdoorKilling() = 0;
        virtual float GetOutdoorKillingMale() = 0;
        virtual float GetSugarFeedKilling() = 0;
        virtual float GetOviTrapKilling(VectorHabitatType::Enum) = 0;
        virtual float GetAnimalFeedKilling() = 0;
        virtual float GetOutdoorRestKilling() = 0;
    };


//#ifdef ENABLE_HIV
    struct IHIVDrugEffects : ISupports
    {
        virtual float GetDrugInactivationRate() = 0;
        virtual float GetDrugClearanceRate() = 0;
        virtual ~IHIVDrugEffects() { }
    };
//#endif

#ifdef ENABLE_POLIO
    struct IPolioVaccine;

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
#endif

    struct IInterventionConsumer : ISupports
    {
        virtual bool GiveIntervention( IDistributableIntervention * pIV ) = 0;
    };

    struct IDMAPI INodeInterventionConsumer : ISupports
    {
        virtual bool GiveIntervention( INodeDistributableIntervention * pIV ) = 0;
    };

    struct IDMAPI IBaseIntervention : ISupports
    {
        virtual float GetCostPerUnit() const = 0;
    };

    struct IIndividualHumanEventContext;

    // TODO - BaseInterventions looks concrete, but can't be instantiated. :(
    struct IDMAPI BaseIntervention : IDistributableIntervention, IBaseIntervention, JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        virtual float GetCostPerUnit() const override { return cost_per_unit; }
        virtual bool Expired() override ;
        virtual void ValidateSimType( const std::string& simTypeStr ) override;

    protected:
        BaseIntervention();
        BaseIntervention( const BaseIntervention& );
        virtual ~BaseIntervention();
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pICCO ) override;

        static void serialize( IArchive& ar, BaseIntervention* obj );

        float cost_per_unit;
        bool expired;
    };

    struct BaseNodeIntervention : IBaseIntervention, JsonConfigurable, INodeDistributableIntervention
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        virtual float GetCostPerUnit() const override { return cost_per_unit; }
        virtual bool Expired();
        virtual void ValidateSimType( const std::string& simTypeStr ) override;

    protected:
        BaseNodeIntervention();
        virtual bool Distribute(INodeEventContext *context, IEventCoordinator2* pEC = nullptr ) override;

        float cost_per_unit;
        bool expired;

#if 0
    private:
        template<class Archive>
        void serialize_inner(Archive &ar, const unsigned int v)
        {
            ar & cost_per_unit;
            ar & expired;
        }
#endif
    };

    struct IDMAPI IIndividualEventObserver : ISupports
    {
        virtual ~IIndividualEventObserver() { }; // for cleanup via interface pointer
        virtual bool notifyOnEvent(IIndividualHumanEventContext *context, const std::string& StateChange) = 0;
    };

    // We're not liking these names anymore. TODO: Change to something more semantically useful
    struct IDMAPI INodeTriggeredInterventionConsumer : ISupports
    {
        virtual void RegisterNodeEventObserver(IIndividualEventObserver* NodeEventObserver, const IndividualEventTriggerType::Enum &trigger ) = 0;
        virtual void UnregisterNodeEventObserver(IIndividualEventObserver* NodeEventObserver, const IndividualEventTriggerType::Enum &trigger ) = 0;
        virtual void TriggerNodeEventObservers(IIndividualHumanEventContext* pIndiv, const IndividualEventTriggerType::Enum &StateChange) = 0;
        virtual void RegisterNodeEventObserverByString( IIndividualEventObserver *pIEO, const std::string &trigger ) = 0;
        virtual void UnregisterNodeEventObserverByString( IIndividualEventObserver *pIEO, const std::string &trigger ) = 0;
        virtual void TriggerNodeEventObserversByString( IIndividualHumanEventContext *ihec, const std::string &trigger ) = 0;
    };
}
