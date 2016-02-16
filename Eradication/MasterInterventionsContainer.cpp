/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "MasterInterventionsContainer.h"
#include "TBInterventionsContainer.h"
#include "HIVInterventionsContainer.h"

#include "InterventionFactory.h"
#include "Log.h"
#include "Sugar.h"

static const char * _module = "MasterInterventionsContainer";

namespace Kernel
{
    Kernel::QueryResult MasterInterventionsContainer::QueryInterface( iid_t iid, void** ppinstance )
    {
        assert(ppinstance); // todo: add a real message: "QueryInterface requires a non-NULL destination!");

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface = nullptr;

        if ( iid == GET_IID(ITBDrugEffects)) 
        {
            for (auto container : InterventionsContainerList)
            {
                if (container->QueryInterface(GET_IID(ITBDrugEffects), (void**)&foundInterface ) == s_OK)
                {
                    break; 
                }
            }
        }
        else if (iid == GET_IID(ITBDrugEffectsApply))
        {
            for (auto container : InterventionsContainerList)
            {
                if (container->QueryInterface(GET_IID(ITBDrugEffectsApply), (void**)&foundInterface ) == s_OK)
                {
                    break; 
                }
            }
        }    
        else if (iid == GET_IID(IHIVDrugEffects))
        {
            for (auto container : InterventionsContainerList)
            {
                if (container->QueryInterface(GET_IID(IHIVDrugEffects), (void**)&foundInterface ) == s_OK)
                {
                    break; 
                }
            }
        }    
        else if (iid == GET_IID(IHIVDrugEffectsApply))
        {
            for (auto container : InterventionsContainerList)
            {
                if (container->QueryInterface(GET_IID(IHIVDrugEffectsApply), (void**)&foundInterface ) == s_OK)
                {
                    break; 
                }
            }
        }
        else if (iid == GET_IID(IInterventionConsumer))
        {
            foundInterface = static_cast<IInterventionConsumer*>(this);
        }

        QueryResult status = e_NOINTERFACE;
        if ( !foundInterface )
        {
            //check the base class
            for (auto container : InterventionsContainerList)
            {
                //first check if it is TB or HIV container, only use the TB container for base class functions
                ISupports* tempInterface;
                if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
                {
                    status = container->QueryInterface(iid, (void**)&foundInterface);
                    if (status == s_OK)
                    {
                        break; 
                    }
                }
            }

            //status = InterventionsContainer::QueryInterface(iid, (void**)&foundInterface);
            // TODO! QI down into the base class, this automatically takes the first off the interventions container list and QIs down to its base class????
            // What to do about repeats of the base class + Master has its own Base class
            
        }
        else
        {
            //foundInterface->AddRef();           // not implementing this yet!
            status = s_OK;
        }

        *ppinstance = foundInterface;
        return status;
    }

    MasterInterventionsContainer::MasterInterventionsContainer() :
        InterventionsContainer()

    {
        InterventionsContainerList.push_back (_new_ TBInterventionsContainer());
        InterventionsContainerList.push_back (_new_ HIVInterventionsContainer());
    }

    MasterInterventionsContainer::~MasterInterventionsContainer()
    {
    }

    void MasterInterventionsContainer::SetContextTo( IIndividualHumanContext* context)
    {
        for (auto container : InterventionsContainerList)
        {
            container->SetContextTo(context);
        }

        //set our own context
        parent = context;
    }

    IIndividualHumanContext* MasterInterventionsContainer::GetParent()
    {
        return parent;
    }

    std::list<IDistributableIntervention*> MasterInterventionsContainer::GetInterventionsByType(const std::string& type_name)
    {
        for (auto container : InterventionsContainerList)
        {
            //first check if it is TB or HIV container, only use the TB container for base class functions
            ISupports* tempInterface;
            if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
            {
                return container->GetInterventionsByType(type_name);
            }
        }

        std::list<IDistributableIntervention*> emptyList;
        return emptyList;
    }
    
    void MasterInterventionsContainer::PurgeExisting( const std::string& iv_name )
    {
        throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__, "MasterInterventionsContainer::PurgeExisting should never be called (for now)." );
    }

    void MasterInterventionsContainer::Update(float dt)
    {
        for (auto container : InterventionsContainerList)
        {
            container->Update(dt);
        }
    }

    void MasterInterventionsContainer::UpdateVaccineAcquireRate( float acq )
    {
        for (auto container : InterventionsContainerList)
        {
            //do for both TB and HIV
            //first check if it is TB or HIV container, only use the TB container for base class functions
//            ISupports* tempInterface;
//            if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
//            {
                container->UpdateVaccineAcquireRate(acq);
//            }
        }    
    }

    void MasterInterventionsContainer::UpdateVaccineTransmitRate( float xmit )
    {
        for (auto container : InterventionsContainerList)
        {
            //do for both TB and HIV
            //first check if it is TB or HIV container, only use the TB container for base class functions
//            ISupports* tempInterface;
//            if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
//            {
                container->UpdateVaccineTransmitRate(xmit);
//            }
        }    
    }

    void MasterInterventionsContainer::UpdateVaccineMortalityRate( float mort )
    {
        for (auto container : InterventionsContainerList)
        {
            //do for both TB and HIV
            //first check if it is TB or HIV container, only use the TB container for base class functions
//            ISupports* tempInterface;
//            if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
//            {
                container->UpdateVaccineMortalityRate(mort);
//            }
        }    
    }

    float MasterInterventionsContainer::GetInterventionReducedAcquire()
    {
        for (auto container : InterventionsContainerList)
        {
            //first check if it is TB or HIV container, only use the TB container for base class functions
            ISupports* tempInterface;
            if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
            {
                return container->GetInterventionReducedAcquire();
            }
        }

        return 0.0f;
    }

    float MasterInterventionsContainer::GetInterventionReducedTransmit()
    {
        for (auto container : InterventionsContainerList)
        {
            //first check if it is TB or HIV container, only use the TB container for base class functions
            ISupports* tempInterface;
            if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
            {
                return container->GetInterventionReducedTransmit();
            }
        }

        return 0.0f;
    }

    float MasterInterventionsContainer::GetInterventionReducedMortality()
    {
        for (auto container : InterventionsContainerList)
        {
            ISupports* tempInterface;
            if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
            {
                return container->GetInterventionReducedMortality();
            }
        }

        return 0.0f;
    }

    bool MasterInterventionsContainer::GiveIntervention( IDistributableIntervention * pIV )
    {
        IHIVDrugEffects* phivde;
        if ( pIV->QueryInterface(GET_IID(IHIVIntervention), (void**)&phivde ) == s_OK)
        {
            for (auto container : InterventionsContainerList)
            {
                ISupports* tempInterface;
                if (container->QueryInterface(GET_IID(IHIVInterventionsContainer), (void**)&tempInterface ) == s_OK)
                {
                    return container->GiveIntervention(pIV);
                }
            }
        }
        else  //default is to go to TBInterventionConsumer 
        {
            for (auto container : InterventionsContainerList)
            {
                ISupports* tempInterface;
                if (container->QueryInterface(GET_IID(ITBInterventionsContainer), (void**)&tempInterface ) == s_OK)
                {
                    return container->GiveIntervention(pIV);
                }
            }
        }

        return false;
    }
}
