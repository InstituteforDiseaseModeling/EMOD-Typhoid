/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Bednet.h"

#include <typeinfo>

#include "Contexts.h"                      // for IIndividualHumanContext, IIndividualHumanInterventionsContext
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainer.h"  // for IBednetConsumer methods

static const char* _module = "SimpleBednet";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(SimpleBednet)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleBednet)

    IMPLEMENT_FACTORY_REGISTERED(SimpleBednet)
    
    SimpleBednet::SimpleBednet( const SimpleBednet& master )
    : BaseIntervention( master )
    {
        killing_config = master.killing_config;
        killing_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( killing_config._json ) );
        blocking_config = master.blocking_config;
        blocking_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( blocking_config._json ) );
    }

    SimpleBednet::SimpleBednet()
    : killing_effect( nullptr )
    , blocking_effect( nullptr )
    {
        initSimTypes( 2, "MALARIA_SIM", "VECTOR_SIM" );
        initConfigTypeMap( "Cost_To_Consumer", &cost_per_unit, SB_Cost_To_Consumer_DESC_TEXT, 0, 999999, 3.75 );
    }

    bool
    SimpleBednet::Configure(
        const Configuration * inputJson
    )
    {
        initConfig( "Bednet_Type", bednet_type, inputJson, MetadataDescriptor::Enum("Bednet_Type", SB_Bednet_Type_DESC_TEXT, MDD_ENUM_ARGS(BednetType)) );
        initConfigComplexType("Killing_Config",  &killing_config, IVM_Killing_Config_DESC_TEXT );
        initConfigComplexType("Blocking_Config",  &blocking_config, "TBD" /*IVM_Blocking_Config_DESC_TEXT*/ );
        bool configured = JsonConfigurable::Configure( inputJson );
        if( !JsonConfigurable::_dryrun )
        {
            killing_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( killing_config._json ) );
            blocking_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( blocking_config._json ) );
        }
        return configured;
    }

    bool
    SimpleBednet::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * const pCCO
    )
    {
        if (s_OK != context->QueryInterface(GET_IID(IBednetConsumer), (void**)&ibc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IBednetConsumer", "IIndividualHumanInterventionsContext" );
        }
        context->PurgeExisting( typeid(*this).name() );
        return BaseIntervention::Distribute( context, pCCO );
    }

    void SimpleBednet::Update( float dt )
    {
        killing_effect->Update(dt);
        blocking_effect->Update(dt);
        float current_killingrate = killing_effect->Current();
        float current_blockingrate = blocking_effect->Current();
        ibc->UpdateProbabilityOfKilling( current_killingrate );
        ibc->UpdateProbabilityOfBlocking( current_blockingrate );
    }

    void SimpleBednet::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IBednetConsumer), (void**)&ibc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IBednetConsumer", "IIndividualHumanContext" );
        }
    }


/*
    Kernel::QueryResult SimpleBednet::QueryInterface( iid_t iid, void **ppinstance )
    {
        assert(ppinstance);

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface;

        if ( iid == GET_IID(IBednet)) 
            foundInterface = static_cast<IBednet*>(this);
        // -->> add support for other I*Consumer interfaces here <<--      
        else if ( iid == GET_IID(ISupports)) 
            foundInterface = static_cast<ISupports*>(static_cast<IBednet*>(this));
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

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::SimpleBednet)

namespace Kernel {
    template<class Archive>
    void serialize(Archive &ar, SimpleBednet& bn, const unsigned int v)
    {
        //LOG_DEBUG("(De)serializing SimpleHousingBednet\n");

        boost::serialization::void_cast_register<SimpleBednet, IDistributableIntervention>();
        ar & bn.bednet_type;
        ar & bn.blocking_effect;
        ar & bn.killing_effect;
        ar & boost::serialization::base_object<Kernel::BaseIntervention>(bn);
    }
}
#endif
