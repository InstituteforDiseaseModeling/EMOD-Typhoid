/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once

#ifdef ENABLE_PYTHON
#include <string>
#include <list>
#include <vector>

#include "Drugs.h"
#include "Interventions.h"
#include "InterventionsContainer.h"
#include "SimpleTypemapRegistration.h"

namespace Kernel
{
    // this container becomes a help implementation member of the relevant IndividualHuman class 
    // it needs to implement consumer interfaces for all the relevant intervention types
    struct IPyDemoDrugEffectsApply : public ISupports
    {
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ) = 0;
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ) = 0;
    };

    class IPyDemoVaccine;

    class PyDemoInterventionsContainer : public InterventionsContainer,
                                          //public IPyDemoDrugEffects,
                                          public IPyDemoDrugEffectsApply
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        PyDemoInterventionsContainer();
        virtual ~PyDemoInterventionsContainer();

        virtual QueryResult QueryInterface(iid_t iid, void** pinstance);

        // IVaccineConsumer: not any more!
        virtual bool GiveIntervention( IDistributableIntervention * pIV );

        // IPyDemoDrugEffectsApply
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ); // not used for anything
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ); // not used for anything

        //IPyDemoDrugEffects(Get)

        virtual void Update(float dt); // example of intervention timestep update

    protected:
        void GiveDrug(IDrug* drug);

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class ::boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, PyDemoInterventionsContainer& cont, const unsigned int v);
#endif
    };
}
#endif
