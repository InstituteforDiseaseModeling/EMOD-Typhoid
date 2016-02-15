/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "HousingModification.h"

#include <typeinfo>

#include "Contexts.h"                      // for IIndividualHumanContext, IIndividualHumanInterventionsContext
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainer.h"  // for IHousingModificationConsumer methods

static const char* _module = "SimpleHousingModification";

namespace Kernel
{
    IMPLEMENT_FACTORY_REGISTERED(SimpleHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(IRSHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(ScreeningHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(SpatialRepellentHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(ArtificialDietHousingModification)
    IMPLEMENT_FACTORY_REGISTERED(InsectKillingFenceHousingModification)

    bool
    SimpleHousingModification::Configure(
        const Configuration * inputJson
    )
    {
        initConfigComplexType("Killing_Config",  &killing_config, IVM_Killing_Config_DESC_TEXT );
        initConfigComplexType("Blocking_Config",  &blocking_config, "TBD" /*IVM_Blocking_Config_DESC_TEXT*/ );
        bool configured = JsonConfigurable::Configure( inputJson );
        if( !JsonConfigurable::_dryrun )
        {
            killing_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( killing_config._json ) );
            blocking_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( blocking_config._json ) );
        }
        return JsonConfigurable::Configure( inputJson );
    }

    SimpleHousingModification::SimpleHousingModification()
        : killing_effect( nullptr )
        , blocking_effect( nullptr )
    {
        initSimTypes( 2, "VECTOR_SIM", "MALARIA_SIM" );
        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, HM_Cost_To_Consumer_DESC_TEXT, 0, 999999, 8.0);
    }

    SimpleHousingModification::SimpleHousingModification( const SimpleHousingModification& master )
    : BaseIntervention( master )
    {
        killing_config = master.killing_config;
        killing_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( killing_config._json ) );
        blocking_config = master.blocking_config;
        blocking_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( blocking_config._json ) );
    }

    bool
    SimpleHousingModification::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * const pCCO
    )
    {
        if (s_OK != context->QueryInterface(GET_IID(IHousingModificationConsumer), (void**)&ihmc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IHousingModificationConsumer", "IIndividualHumanInterventionsContext" );
        }
        context->PurgeExisting( typeid(*this).name() );
        return BaseIntervention::Distribute( context, pCCO );
    }

    void SimpleHousingModification::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        LOG_DEBUG("SimpleHousingModification::SetContextTo (probably deserializing)\n");
        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IHousingModificationConsumer), (void**)&ihmc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IHousingModificationConsumer", "IIndividualHumanContext" );
        }

    }

    void SimpleHousingModification::Update( float dt )
    {
        killing_effect->Update(dt);
        blocking_effect->Update(dt);
        float current_killingrate = killing_effect->Current();
        float current_blockingrate = blocking_effect->Current();

        if( ihmc )
        {
            ihmc->ApplyHouseBlockingProbability( current_blockingrate );
            ihmc->UpdateProbabilityOfScreenKilling( current_killingrate );
        }
        else
        {
            throw NullPointerException( __FILE__, __LINE__, __FUNCTION__, "ihmc" );
        }
    }

    BEGIN_QUERY_INTERFACE_BODY(SimpleHousingModification)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleHousingModification)
/*
    Kernel::QueryResult SimpleHousingModification::QueryInterface( iid_t iid, void **ppinstance )
    {
        assert(ppinstance);

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface;

        if ( iid == GET_IID(IHousingModification)) 
            foundInterface = static_cast<IHousingModification*>(this);
        // -->> add support for other I*Consumer interfaces here <<--      
      
        else if ( iid == GET_IID(ISupports)) 
            foundInterface = static_cast<ISupports*>(static_cast<IHousingModification*>(this));
        else
            foundInterface = 0;

        QueryResult status;
        if ( !foundInterface )
            status = e_NOINTERFACE;
        else
        {
            //foundInterface->AddRef();           // not implementing this yet!
            status = s_OK;
        }

        *ppinstance = foundInterface;
        return status;
    }*/
}

// This shows how to do serialization from outside the class.
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::SimpleHousingModification)
BOOST_CLASS_EXPORT(Kernel::IRSHousingModification)
BOOST_CLASS_EXPORT(Kernel::ScreeningHousingModification)
BOOST_CLASS_EXPORT(Kernel::SpatialRepellentHousingModification)
BOOST_CLASS_EXPORT(Kernel::ArtificialDietHousingModification)
BOOST_CLASS_EXPORT(Kernel::InsectKillingFenceHousingModification)

namespace Kernel {

    template<class Archive>
    void serialize(Archive &ar, SimpleHousingModification& hm, const unsigned int v)
    {
        static const char * _module = "SimpleHousingModification";
        LOG_DEBUG("(De)serializing SimpleHousingModification\n");

        ar & hm.killing_effect;
        ar & hm.blocking_effect;
        ar & boost::serialization::base_object<BaseIntervention>(hm);
    }

    template<class Archive>
    void serialize(Archive &ar, IRSHousingModification& hm, const unsigned int v)
    {
        boost::serialization::void_cast_register<IRSHousingModification, IDistributableIntervention>();
        ar & boost::serialization::base_object<SimpleHousingModification>(hm);
    }

    template<class Archive>
    void serialize(Archive &ar, ScreeningHousingModification& hm, const unsigned int v)
    {
        boost::serialization::void_cast_register<ScreeningHousingModification, IDistributableIntervention>();
        ar & boost::serialization::base_object<SimpleHousingModification>(hm);
    }

    template<class Archive>
    void serialize(Archive &ar, SpatialRepellentHousingModification& hm, const unsigned int v)
    {
        boost::serialization::void_cast_register<SpatialRepellentHousingModification, IDistributableIntervention>();
        ar & boost::serialization::base_object<SimpleHousingModification>(hm);
    }

    template<class Archive>
    void serialize(Archive &ar, ArtificialDietHousingModification& hm, const unsigned int v)
    {
        boost::serialization::void_cast_register<ArtificialDietHousingModification, IDistributableIntervention>();
        ar & boost::serialization::base_object<SimpleHousingModification>(hm);
    }

    template<class Archive>
    void serialize(Archive &ar, InsectKillingFenceHousingModification& hm, const unsigned int v)
    {
        boost::serialization::void_cast_register<InsectKillingFenceHousingModification, IDistributableIntervention>();
        ar & boost::serialization::base_object<SimpleHousingModification>(hm);
    }
}
#endif

