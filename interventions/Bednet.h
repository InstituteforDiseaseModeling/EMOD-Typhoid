/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "BoostLibWrapper.h"

#include "Interventions.h"
#include "SimpleTypemapRegistration.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "FactorySupport.h"
#include "Configure.h"
#include "WaningEffect.h"

namespace Kernel
{
    struct IBednetConsumer; 

    /* Keep around as an identity solution??? */
    struct IBednet : public ISupports
    {
    };

    class SimpleBednet : public BaseIntervention
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, SimpleBednet, IDistributableIntervention)

    public:
        SimpleBednet();
        SimpleBednet( const SimpleBednet& );
        virtual ~SimpleBednet() { }

        bool Configure( const Configuration * config );

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO );
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void SetContextTo(IIndividualHumanContext *context);
        virtual void Update(float dt);

    protected:
        WaningConfig   killing_config;
        IWaningEffect* killing_effect;
        WaningConfig   blocking_config;
        IWaningEffect* blocking_effect;
        BednetType::Enum bednet_type;
        IBednetConsumer *ibc;

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class ::boost::serialization::access;
        template<typename Archive>
        friend void serialize( Archive &ar, SimpleBednet& bn, unsigned int version );
#endif
    };
}
