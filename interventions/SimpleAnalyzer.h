/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "InterventionFactory.h"
#include "Interventions.h"
#include "InterventionEnums.h"
#include "Configuration.h"
#include "Configure.h"
#include "SimpleTypemapRegistration.h"

namespace Kernel
{
    class SimpleAnalyzer : public IIndividualEventObserver, 
                           public BaseNodeIntervention
    {
    protected:
        // ctor
        SimpleAnalyzer();

    public:
        // dtor
        virtual ~SimpleAnalyzer();

        // InterventionFactory
        DECLARE_FACTORY_REGISTERED(InterventionFactory, SimpleAnalyzer, INodeDistributableIntervention) 

        // JsonConfigurable
        DECLARE_CONFIGURED(SimpleAnalyzer)

        // INodeDistributableIntervention
        virtual bool Distribute(INodeEventContext *context, IEventCoordinator2* pEC=NULL); 
        virtual void SetContextTo(INodeEventContext *context);
        virtual void Update(float dt);

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

        // IIndividualEventObserver
        virtual bool notifyOnEvent( IIndividualHumanEventContext *context, const std::string& StateChange );
        virtual std::string GetTriggerCondition() const;

#if USE_JSON_SERIALIZATION || USE_JSON_MPI
        void JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const;
        void JDeserialize( IJsonObjectAdapter* root, JSerializer* helper );
#endif

    protected:
        float  m_timer;
        float  m_reporting_interval;
        float  m_coverage;
        IndividualEventTriggerType::Enum  m_trigger_condition;

        INodeEventContext * m_parent;

    private: 
#if USE_BOOST_SERIALIZATION
        // serialization
        friend class ::boost::serialization::access; 
        template<class Archive> 
        friend void serialize(Archive &ar, SimpleAnalyzer& ba, const unsigned int v); 
#endif
    };
}

// Serialize SimpleAnalyzer outside of class declaration
#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(Kernel::SimpleAnalyzer)

namespace Kernel {
    REGISTER_SERIALIZATION_VOID_CAST(SimpleAnalyzer, INodeDistributableIntervention);
    REGISTER_SERIALIZATION_VOID_CAST(SimpleAnalyzer, IIndividualEventObserver);
    template<class Archive>
    void serialize(Archive &ar, SimpleAnalyzer& obj, const unsigned int v)
    {
        ar & obj.m_timer;
        ar & obj.m_reporting_interval;
        ar & obj.m_trigger_condition;
    }
}
#endif