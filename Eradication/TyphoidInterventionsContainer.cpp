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
#ifdef ENABLE_TYPHOID

#include "Sugar.h"
#include "Debug.h"
#include "Environment.h"

#include "Drugs.h"

#include "Contexts.h"
#include "InterventionFactory.h"
#include "TyphoidInterventionsContainer.h"


namespace Kernel
{
    static const char* _module = "TyphoidInterventionsContainer";

    BEGIN_QUERY_INTERFACE_DERIVED(TyphoidInterventionsContainer, InterventionsContainer)
        //HANDLE_INTERFACE(ITyphoidVaccineEffects)
        //HANDLE_INTERFACE(ITyphoidDrugEffects)
        HANDLE_INTERFACE(ITyphoidDrugEffectsApply)
    END_QUERY_INTERFACE_DERIVED(TyphoidInterventionsContainer, InterventionsContainer)

    TyphoidInterventionsContainer::TyphoidInterventionsContainer()
    {
    }

    TyphoidInterventionsContainer::~TyphoidInterventionsContainer()
    {
    }

    void TyphoidInterventionsContainer::Update(float dt)
    {
        // call base level
        InterventionsContainer::Update(dt);
    }

    bool TyphoidInterventionsContainer::GiveIntervention( IDistributableIntervention * pIV )
    {
        pIV->SetContextTo( parent );

        // Additionally, keep this newly-distributed polio-vaccine pointer in the 'new_vaccines' list.  
        // IndividualHuman::applyNewInterventionEffects will come looking for these to apply to SusceptibilityTyphoid.
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

    void TyphoidInterventionsContainer::GiveDrug(IDrug* drug)
    {
        drug->ConfigureDrugTreatment();
    }

    void TyphoidInterventionsContainer::ApplyDrugVaccineReducedAcquireEffect( float rate )
    {
    }
    
    void TyphoidInterventionsContainer::ApplyDrugVaccineReducedTransmitEffect( float rate )
    {
    }

}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::TyphoidInterventionsContainer)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive &ar, TyphoidInterventionsContainer& cont, const unsigned int v)
    {
        static const char * _module = "TyphoidInterventionsContainer";
        LOG_DEBUG("(De)serializing TyphoidInterventionsContainer\n");

        //ar & cont.new_vaccines;  // SusceptibilityTyphoid update based on new vaccines done in same time step?
        boost::serialization::void_cast_register<TyphoidInterventionsContainer, InterventionsContainer>();
        ar & boost::serialization::base_object<Kernel::InterventionsContainer>(cont);
    }
}
#endif

#endif // ENABLE_TYPHOID
