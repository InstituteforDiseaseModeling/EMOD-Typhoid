/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "STIInterventionsContainer.h"

#include "IIndividualHuman.h"

//#include "Drugs.h" // for IDrug interface
#include "SimulationEnums.h"
#include "InterventionFactory.h"
#include "Log.h"
#include "Sugar.h"
#include "IndividualSTI.h"
#include "IRelationshipParameters.h"

static const char * _module = "STIInterventionsContainer";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(STIInterventionsContainer, InterventionsContainer)
        HANDLE_INTERFACE(ISTIInterventionsContainer)
        HANDLE_INTERFACE(ISTIBarrierConsumer)
        HANDLE_INTERFACE(ISTICircumcisionConsumer)
        HANDLE_INTERFACE(ISTICoInfectionStatusChangeApply)
    END_QUERY_INTERFACE_DERIVED(STIInterventionsContainer, InterventionsContainer)

    STIInterventionsContainer::STIInterventionsContainer() :
        InterventionsContainer(),
        is_circumcised(false)
    {
    }

    STIInterventionsContainer::~STIInterventionsContainer()
    {
    }
    
    void STIInterventionsContainer::Update(float dt)
    {
        InterventionsContainer::Update(dt);
    }

    // For now, before refactoring Drugs to work in new way, just check if the intervention is a
    // Drug, and if so, add to drugs list. In future, there will be no drugs list, just interventions.
    bool STIInterventionsContainer::GiveIntervention(
        IDistributableIntervention * pIV
    )
    {
        bool ret = true;

        // NOTE: Calling this AFTER the QI/GiveDrug crashes!!! Both win and linux. Says SetContextTo suddenly became a pure virtual.

        ICircumcision * pCirc = nullptr;
        if( s_OK == pIV->QueryInterface(GET_IID(ICircumcision), (void**) &pCirc) )
        {
            LOG_DEBUG("Getting circumcised\n");
            ret = ret && ApplyCircumcision(pCirc);
        }

        return ret && InterventionsContainer::GiveIntervention( pIV );
    }

    void
    STIInterventionsContainer::UpdateSTIBarrierProbabilitiesByType(
        RelationshipType::Enum rel_type,
        const Sigmoid& config_overrides
    )
    {
        STI_blocking_overrides[ rel_type ] = config_overrides;
    }

    const Sigmoid&
    STIInterventionsContainer::GetSTIBarrierProbabilitiesByRelType(
        const IRelationshipParameters* pRelParams
    )
    const
    {
        if( STI_blocking_overrides.find( pRelParams->GetType() ) != STI_blocking_overrides.end() )
        {
            LOG_DEBUG( "Using override condom config values from campaign.\n" );
            return STI_blocking_overrides.at( pRelParams->GetType() );
        }
        else
        {
            LOG_DEBUG( "Using static (default/config.json) condom config values.\n" );
            return pRelParams->GetCondomUsage();
        }
    }

    float
    STIInterventionsContainer::GetInterventionReducedAcquire()
    const
    {
        return drugVaccineReducedAcquire;
    }

    float
    STIInterventionsContainer::GetInterventionReducedTransmit()
    const
    {
        return drugVaccineReducedTransmit;
    }

    bool STIInterventionsContainer::IsCircumcised( void ) const {
        return is_circumcised;
    }

    bool STIInterventionsContainer::ApplyCircumcision( ICircumcision *pCirc ) {
        // Need to get gender
        IIndividualHuman *ih = nullptr;
        if( s_OK != parent->QueryInterface(GET_IID(IIndividualHuman), (void**) &ih) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHuman", "IIndividualHuman" );
        }

        if( ih->GetGender() == Gender::FEMALE )
        {
            return false;
        }

        if( IsCircumcised() )
        {
            return false;
        }

        is_circumcised = true;
        return true;
    }

    void STIInterventionsContainer::ChangeProperty( const char *prop, const char* new_value)
    {
        IIndividualHumanSTI *ihsti = nullptr;
        if( s_OK != parent->QueryInterface(GET_IID(IIndividualHumanSTI), (void**) &ihsti) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHumanContext", "IIndividualHumanSTI" );
        }
        ihsti->UpdateSTINetworkParams(prop, new_value);

        InterventionsContainer::ChangeProperty( prop, new_value);
    }

    void
    STIInterventionsContainer::SpreadStiCoInfection()
    {
        IIndividualHumanSTI *ihsti = nullptr;
        if( s_OK != parent->QueryInterface(GET_IID(IIndividualHumanSTI), (void**) &ihsti) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHumanContext", "IIndividualHumanSTI" );
        }
        ihsti->SetStiCoInfectionState();
    }

    void
    STIInterventionsContainer::CureStiCoInfection()
    {
        IIndividualHumanSTI *ihsti = nullptr;
        if( s_OK != parent->QueryInterface(GET_IID(IIndividualHumanSTI), (void**) &ihsti) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHumanContext", "IIndividualHumanSTI" );
        }
        ihsti->ClearStiCoInfectionState();
    }

    REGISTER_SERIALIZABLE(STIInterventionsContainer);

    void serialize_overrides( IArchive& ar, std::map< RelationshipType::Enum, Sigmoid >& blocking_overrides )
    {
        size_t count = ar.IsWriter() ? blocking_overrides.size() : -1;

        ar.startArray(count);
        if (ar.IsWriter())
        {
            for (auto& entry : blocking_overrides)
            {
                std::string key = RelationshipType::pairs::lookup_key( entry.first );
                ar.startObject();
                    ar.labelElement("key"  ) & key;
                    ar.labelElement("value") & entry.second;
                ar.endObject();
            }
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                std::string key;
                Sigmoid value;
                ar.startObject();
                    ar.labelElement("key"  ) & key;
                    ar.labelElement("value") & value;
                ar.endObject();
                RelationshipType::Enum rt = (RelationshipType::Enum)RelationshipType::pairs::lookup_value( key.c_str() );
                blocking_overrides[ rt ] = value;
            }
        }
        ar.endArray();
    }

    void STIInterventionsContainer::serialize(IArchive& ar, STIInterventionsContainer* obj)
    {
        InterventionsContainer::serialize( ar, obj );
        STIInterventionsContainer& container = *obj;
        ar.labelElement("is_circumcised") & container.is_circumcised;
        ar.labelElement("STI_blocking_overrides"); serialize_overrides( ar, container.STI_blocking_overrides );
    }
}

#if 0
namespace Kernel {
    template<class Archive>
    void serialize(Archive &ar, STIInterventionsContainer& container, const unsigned int v)
    {
        ar & container.is_circumcised;

        ar & boost::serialization::base_object<InterventionsContainer>(container);
    }
}
#endif
