/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#ifdef ENABLE_POLIO

#include "SimpleTypemapRegistration.h"
#include "Sugar.h"
#include "Debug.h"
#include "Environment.h"

#include "Drugs.h"

#include "Contexts.h"
#include "InterventionFactory.h"
#include "PolioInterventionsContainer.h"
#include "PolioVaccine.h"
#include <stdexcept>


namespace Kernel
{
    static const char* _module = "PolioInterventionsContainer";

    BEGIN_QUERY_INTERFACE_DERIVED(PolioInterventionsContainer, InterventionsContainer)
        HANDLE_INTERFACE(IPolioVaccineEffects)
        HANDLE_INTERFACE(IPolioDrugEffects)
        HANDLE_INTERFACE(IPolioDrugEffectsApply)
    END_QUERY_INTERFACE_DERIVED(PolioInterventionsContainer, InterventionsContainer)

    PolioInterventionsContainer::PolioInterventionsContainer()
        : titer_efficacy(0.0f)
        , infection_duration_efficacy(0.0f)
    {
    }

    PolioInterventionsContainer::~PolioInterventionsContainer()
    {
    }

    void PolioInterventionsContainer::Update(float dt)
    {
        titer_efficacy = 0.0f;
        infection_duration_efficacy = 0.0f;

        // call base level
        InterventionsContainer::Update(dt);
    }

    bool PolioInterventionsContainer::GiveIntervention( IDistributableIntervention * pIV )
    {
        pIV->SetContextTo( parent );

        // Additionally, keep this newly-distributed polio-vaccine pointer in the 'new_vaccines' list.  
        // IndividualHuman::applyNewInterventionEffects will come looking for these to apply to SusceptibilityPolio.
        IPolioVaccine* ipvac = nullptr;
        if (s_OK == pIV->QueryInterface(GET_IID(IPolioVaccine), (void**)&ipvac) )
        {
            new_vaccines.push_front(ipvac);
        }

        IDrug * pDrug = nullptr;
        if( s_OK == pIV->QueryInterface(GET_IID(IDrug), (void**) &pDrug) )
        {
            LOG_DEBUG("Getting a drug\n");
            GiveDrug( pDrug );
        } 

        return InterventionsContainer::GiveIntervention(pIV);
    }

    void PolioInterventionsContainer::GiveDrug(IDrug* drug)
    {
        drug->ConfigureDrugTreatment();
    }

    void PolioInterventionsContainer::ApplyDrugTiterEffect( float rate )
    {
        LOG_DEBUG_F("apply_titer effect %f\n", rate);

        titer_efficacy += rate;
    }
    void PolioInterventionsContainer::ApplyDrugDurationEffect( float rate )
    {

        infection_duration_efficacy += rate;
    }

    void PolioInterventionsContainer::ApplyDrugVaccineReducedAcquireEffect( float rate )
    {
    }
    
    void PolioInterventionsContainer::ApplyDrugVaccineReducedTransmitEffect( float rate )
    {
    }

    // getters
    float PolioInterventionsContainer::get_titer_efficacy() const
    {
        LOG_DEBUG_F("getter titer %f\n", titer_efficacy);
        return titer_efficacy;
    }

    float PolioInterventionsContainer::get_infection_duration_efficacy() const
    {
        return infection_duration_efficacy;
    }

    std::list<IPolioVaccine*>&
    PolioInterventionsContainer::GetNewVaccines()
    {
        return new_vaccines;
    }

    void
        PolioInterventionsContainer::ClearNewVaccines()
    {
        new_vaccines.clear();

        for ( std::list<IDistributableIntervention*>::iterator iterator = interventions.begin(); iterator != interventions.end(); )
        {
            std::string cur_iv_type_name = typeid(**iterator).name();
            if( cur_iv_type_name == "class Kernel::PolioVaccine" )
            {
                delete *iterator;
                iterator = interventions.erase(iterator);
            }
            else
            {
                iterator++;
            }
        }
    }

    REGISTER_SERIALIZABLE(PolioInterventionsContainer);

    void PolioInterventionsContainer::serialize(IArchive& ar, PolioInterventionsContainer* obj)
    {
        InterventionsContainer::serialize(ar, obj);
        /* Not needed yet(?)
        PolioInterventionsContainer& interventions = *dynamic_cast<PolioInterventionsContainer*>(obj);
        ar.startObject();
            ar.labelElement("new_vaccines"); ar.serialize(interventions.new_vaccines);
            ar.labelElement("titer_efficacy") & interventions.titer_efficacy;
            ar.labelElement("infection_duration_efficacy") & interventions.infection_duration_efficacy;
        ar.endObject();
        */
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::PolioInterventionsContainer)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive &ar, PolioInterventionsContainer& cont, const unsigned int v)
    {
        static const char * _module = "PolioInterventionsContainer";
        LOG_DEBUG("(De)serializing PolioInterventionsContainer\n");

        //ar & cont.new_vaccines;  // SusceptibilityPolio update based on new vaccines done in same time step?
        boost::serialization::void_cast_register<PolioInterventionsContainer, InterventionsContainer>();
        ar & boost::serialization::base_object<Kernel::InterventionsContainer>(cont);
    }
}
#endif

#endif // ENABLE_POLIO
