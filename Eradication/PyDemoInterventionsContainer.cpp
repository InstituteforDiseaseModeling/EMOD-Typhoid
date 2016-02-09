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

#include "stdafx.h"
#ifdef ENABLE_PYTHON

#include "SimpleTypemapRegistration.h"
#include "Sugar.h"
#include "Debug.h"
#include "Environment.h"

#include "Drugs.h"

#include "Contexts.h"
#include "InterventionFactory.h"
#include "PyDemoInterventionsContainer.h"


namespace Kernel
{
    static const char* _module = "PyDemoInterventionsContainer";

    BEGIN_QUERY_INTERFACE_DERIVED(PyDemoInterventionsContainer, InterventionsContainer)
        //HANDLE_INTERFACE(IPyDemoVaccineEffects)
        //HANDLE_INTERFACE(IPyDemoDrugEffects)
        HANDLE_INTERFACE(IPyDemoDrugEffectsApply)
    END_QUERY_INTERFACE_DERIVED(PyDemoInterventionsContainer, InterventionsContainer)

    PyDemoInterventionsContainer::PyDemoInterventionsContainer()
    {
    }

    PyDemoInterventionsContainer::~PyDemoInterventionsContainer()
    {
    }

    void PyDemoInterventionsContainer::Update(float dt)
    {
        // call base level
        InterventionsContainer::Update(dt);
    }

    bool PyDemoInterventionsContainer::GiveIntervention( IDistributableIntervention * pIV )
    {
        pIV->SetContextTo( parent );

        // Additionally, keep this newly-distributed polio-vaccine pointer in the 'new_vaccines' list.  
        // IndividualHuman::applyNewInterventionEffects will come looking for these to apply to SusceptibilityPyDemo.
        IDistributableIntervention* ipvac = nullptr;
/*
        IDrug * pDrug = NULL;
        if( s_OK == pIV->QueryInterface(GET_IID(IDrug), (void**) &pDrug) )
        {
            LOG_DEBUG("Getting a drug\n");
            GiveDrug( pDrug );
        } 
*/
        return InterventionsContainer::GiveIntervention(pIV);
    }

    void PyDemoInterventionsContainer::GiveDrug(IDrug* drug)
    {
        drug->ConfigureDrugTreatment();
    }

    void PyDemoInterventionsContainer::ApplyDrugVaccineReducedAcquireEffect( float rate )
    {
    }
    
    void PyDemoInterventionsContainer::ApplyDrugVaccineReducedTransmitEffect( float rate )
    {
    }

}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::PyDemoInterventionsContainer)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive &ar, PyDemoInterventionsContainer& cont, const unsigned int v)
    {
        static const char * _module = "PyDemoInterventionsContainer";
        LOG_DEBUG("(De)serializing PyDemoInterventionsContainer\n");

        //ar & cont.new_vaccines;  // SusceptibilityPyDemo update based on new vaccines done in same time step?
        boost::serialization::void_cast_register<PyDemoInterventionsContainer, InterventionsContainer>();
        ar & boost::serialization::base_object<Kernel::InterventionsContainer>(cont);
    }
}
#endif

#endif // ENABLE_PYTHON
