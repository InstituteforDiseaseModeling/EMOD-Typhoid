/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Interventions.h"
#include "SimpleTypemapRegistration.h"
#include "Configuration.h"
#include "InterventionFactory.h"
#include "InterventionEnums.h"
#include "Configure.h" // JsonConfigurable

namespace Kernel
{
    struct IBTNTI; 

    struct IBTNTIConsumer : public ISupports
    {
        virtual void GiveBTNTI(IBTNTI* BTNTI) = 0;
        virtual const IBTNTI* GetBTNTI() = 0; // look at the BTNTI they have...why not, some campaigns may want to introspect on this sort of thing. returns NULL if there is no BTNTI currently
    };

    struct IBTNTI : public INodeDistributableIntervention
    {
        virtual float GetEfficacy() const = 0; 
        virtual ~IBTNTI() { }; // needed for cleanup via interface pointer
    };

    class SimpleBTNTI : public IBTNTI, public BaseNodeIntervention
    {
        DECLARE_SERIALIZABLE(SimpleBTNTI)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_CONFIGURED(SimpleBTNTI)
        DECLARE_FACTORY_REGISTERED(InterventionFactory, SimpleBTNTI, INodeDistributableIntervention) 

    public:        
        virtual ~SimpleBTNTI() { }

        // INodeDistributableIntervention
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        virtual void SetContextTo(INodeEventContext *context) { } // not needed for this intervention
        virtual void Update(float dt);

        // IBTNTI
        virtual float GetEfficacy() const  { return efficacy; }

    protected:

        SimpleBTNTI();

        float efficacy;

    private:

#if 0
        template<class Archive>
        void serialize_inner(Archive &ar, const unsigned int v)
        {
            typemap.serialize(this, ar, v);
            // ar & efficacy; // don't need to persist because this intervention only acts at time of distribution.
        }
#endif
    };
}
