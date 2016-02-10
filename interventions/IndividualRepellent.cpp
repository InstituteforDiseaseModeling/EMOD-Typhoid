/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "IndividualRepellent.h"

#include "Contexts.h"                      // for IIndividualHumanContext, IIndividualHumanInterventionsContext
#include "InterventionEnums.h"
#include "InterventionFactory.h"
#include "VectorInterventionsContainer.h"  // for IIndividualRepellentConsumer methods

static const char* _module = "SimpleIndividualRepellent";

namespace Kernel
{
    IMPLEMENT_FACTORY_REGISTERED(SimpleIndividualRepellent)

    bool
    SimpleIndividualRepellent::Configure(
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
        return configured;
    }

    SimpleIndividualRepellent::SimpleIndividualRepellent()
        : killing_effect( nullptr )
        , blocking_effect( nullptr )
        , current_killingrate( 0.0f )
        , current_blockingrate( 0.0f )
    {
        initSimTypes( 2, "VECTOR_SIM", "MALARIA_SIM" );
        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, SIR_Cost_To_Consumer_DESC_TEXT, 0, 999999, 8.0);
    }

    SimpleIndividualRepellent::SimpleIndividualRepellent( const SimpleIndividualRepellent& master )
    : BaseIntervention( master )
    {
        killing_config = master.killing_config;
        killing_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( killing_config._json ) );
        blocking_config = master.blocking_config;
        blocking_effect = WaningEffectFactory::CreateInstance( Configuration::CopyFromElement( blocking_config._json ) );
    }

    bool
    SimpleIndividualRepellent::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * const pCCO
    )
    {
        if (s_OK != context->QueryInterface(GET_IID(IIndividualRepellentConsumer), (void**)&ihmc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IIndividualRepellentConsumer", "IIndividualHumanInterventionsContext" );
        }
        return BaseIntervention::Distribute( context, pCCO );
    }

    void SimpleIndividualRepellent::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        LOG_DEBUG("SimpleIndividualRepellent::SetContextTo (probably deserializing)\n");
        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IIndividualRepellentConsumer), (void**)&ihmc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IIndividualRepellentConsumer", "IIndividualHumanContext" );
        }
        current_killingrate = killing_effect->Current();
        current_blockingrate = blocking_effect->Current();
    }

    void SimpleIndividualRepellent::Update( float dt )
    {
        killing_effect->Update(dt);
        blocking_effect->Update(dt);
        current_killingrate = killing_effect->Current();
        current_blockingrate = blocking_effect->Current();

        if( ihmc )
        {
            ihmc->UpdateProbabilityOfIndRepBlocking( current_blockingrate );
            ihmc->UpdateProbabilityOfIndRepKilling( current_killingrate );
        }
        else
        {
            // ERROR: ihmc (interventions container) pointer null. Should be impossible to get here, but that's 
            // what one always says about null pointers! :)
            throw NullPointerException( __FILE__, __LINE__, __FUNCTION__, "ihmc", "IIndividualRepellentConsumer" );
        }
    }

    BEGIN_QUERY_INTERFACE_BODY(SimpleIndividualRepellent)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(SimpleIndividualRepellent)
/*
    Kernel::QueryResult SimpleIndividualRepellent::QueryInterface( iid_t iid, void **ppinstance )
    {
        assert(ppinstance);

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface;

        if ( iid == GET_IID(IIndividualRepellent)) 
            foundInterface = static_cast<IIndividualRepellent*>(this);
        // -->> add support for other I*Consumer interfaces here <<--      
      
        else if ( iid == GET_IID(ISupports)) 
            foundInterface = static_cast<ISupports*>(static_cast<IIndividualRepellent*>(this));
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
BOOST_CLASS_EXPORT(Kernel::SimpleIndividualRepellent)

namespace Kernel {

    template<class Archive>
    void serialize(Archive &ar, SimpleIndividualRepellent& obj, const unsigned int v)
    {
        static const char * _module = "SimpleIndividualRepellent";
        LOG_DEBUG("(De)serializing SimpleIndividualRepellent\n");

        boost::serialization::void_cast_register<SimpleIndividualRepellent, IDistributableIntervention>();
        ar & obj.blocking_effect;
        ar & obj.killing_effect;
        ar & boost::serialization::base_object<Kernel::BaseIntervention>(obj);
    }
}
#endif
