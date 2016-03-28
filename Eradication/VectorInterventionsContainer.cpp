/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "VectorInterventionsContainer.h"

#include "Exceptions.h"
#include "InterventionFactory.h"
#include "SimulationConfig.h"  // for "Human_Feeding_Mortality" parameter

static const char* _module = "VectorInterventionsContainer";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(VectorInterventionsContainer, InterventionsContainer)
        HANDLE_INTERFACE(IBednetConsumer)
        HANDLE_INTERFACE(IVectorInterventionsEffects)
        HANDLE_INTERFACE(IHousingModificationConsumer)
        HANDLE_INTERFACE(IIndividualRepellentConsumer)
        HANDLE_INTERFACE(IVectorInterventionEffectsSetter)
    END_QUERY_INTERFACE_DERIVED(VectorInterventionsContainer, InterventionsContainer)

    VectorInterventionsContainer::VectorInterventionsContainer() : 
        InterventionsContainer(), 
        pDieBeforeFeeding(0),
        pHostNotAvailable(0),
        pDieDuringFeeding(.1f),
        pDiePostFeeding(0),
        pSuccessfulFeedAD(0),
        pSuccessfulFeedHuman(.9f),
        pOutdoorDieBeforeFeeding(0),
        pOutdoorHostNotAvailable(0),
        pOutdoorDieDuringFeeding(.1f),
        pOutdoorDiePostFeeding(0),
        pOutdoorSuccessfulFeedHuman(.9f),
        blockIndoorVectorAcquire(1),
        blockIndoorVectorTransmit(.9f),
        blockOutdoorVectorAcquire(1),
        blockOutdoorVectorTransmit(.9f)
    {
    }

    VectorInterventionsContainer::~VectorInterventionsContainer()
    {
    }

    void VectorInterventionsContainer::UpdateProbabilityOfBlocking(
        float prob
    )
    {
        p_block_net = prob;
    }

    void VectorInterventionsContainer::UpdateProbabilityOfKilling(
        float prob
    )
    {
        p_kill_ITN = prob;
    }

    void VectorInterventionsContainer::ApplyHouseBlockingProbability(
        float prob
    )
    {
        p_penetrate_housingmod *= (1.0f-prob);  // will multiply by 1-all housing mods and then do 1- that.
    }

    void VectorInterventionsContainer::UpdateProbabilityOfScreenKilling(
        float prob
    )
    {
        p_kill_IRSpostfeed = prob;
    }

    void VectorInterventionsContainer::UpdatePhotonicFenceKillingRate(
        float rate
    )
    {
        p_kill_PFH = rate;  // killing rate is the effect(to reuse methods), but it actually kills mosquitoes because that is its first and only efficacy
    }
            
    void VectorInterventionsContainer::UpdateArtificialDietAttractionRate(
        float rate
    )
    {
        p_attraction_ADIH = rate;
    }

    void VectorInterventionsContainer::UpdateArtificialDietKillingRate(
        float rate
    )
    {
        p_kill_ADIH = rate;
    }

    void VectorInterventionsContainer::UpdateProbabilityOfIndRepBlocking(
        float prob
        )
    {
        p_block_indrep = prob;
    }

    void VectorInterventionsContainer::UpdateProbabilityOfIndRepKilling(
        float prob
        )
    {
        if ( prob > 0 )
        {
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Individual repellents only currently support blocking but not killing." );
        }

        p_kill_indrep  = prob;
    }

    void VectorInterventionsContainer::UpdateInsecticidalDrugKillingProbability( float prob )
    {
        p_survive_insecticidal_drug *= (1.0f-prob);  // will multiply by 1-all drugs and then do 1- that.
    }

    void VectorInterventionsContainer::Update(float dt)
    {
        // TODO: Re-implement policy of 1 intervention of each type w/o
        // knowing a priori about any intervention types. Current favorite
        // idea is for interventions to enforce this in the Give by doing a
        // Get and Remove first via QI. 
        p_block_net = 0;
        p_kill_ITN = 0;
        p_penetrate_housingmod = 1.0;   // block probability of housing: screening, spatial repellent, IRS repellent-- starts at 1.0 because will do 1.0-p_block_housing below
        p_kill_IRSprefeed = 0;   // pre-feed kill probability of IRS (TODO: this is not hooked up!)
        p_kill_IRSpostfeed = 0;  // probability of IRS killing postfeed
        p_block_indrep = 0;      // probability of individual repellent blocking a feed
        p_kill_indrep = 0;       // probability of individual repellent killing post feed (not used yet)
        p_kill_PFH = 0;          // kill probability of house photonic fence
        p_attraction_ADIH = 0;   // probability of distraction by Artificial Diet--In House
        p_kill_ADIH = 0;         // kill probability of in-house artificial diet
        p_survive_insecticidal_drug = 1.0; // post-feed kill probability of insecticidal drug (e.g. Ivermectin)-- starts at 1.0 because will do 1.0-p_kill below

        float p_dieduringfeeding = GET_CONFIGURABLE(SimulationConfig)->human_feeding_mortality;

        // call base level
        InterventionsContainer::Update(dt);

        // final adjustment to product of (1-prob) accumulated over potentially multiple instances
        float p_block_housing = 1.0f - p_penetrate_housingmod;
        float p_kill_insecticidal_drug = 1.0f - p_survive_insecticidal_drug;
        // range fix
        if(p_block_housing < 0) p_block_housing = 0;
        if(p_kill_insecticidal_drug < 0) p_kill_insecticidal_drug = 0;
        // adjust indoor post-feed kill to include combined probability of IRS & insectidical drug
        p_kill_IRSpostfeed = 1.0f - ((1.0f-p_kill_IRSpostfeed)*(1.0f-p_kill_insecticidal_drug));

        // now get probabilities for indoor feeding outcomes
        pDieBeforeFeeding    = p_kill_PFH+(1-p_kill_PFH)*(1-p_block_housing)*(p_kill_IRSprefeed+(1-p_kill_IRSprefeed)*(p_attraction_ADIH*(p_kill_ADIH+(1-p_kill_ADIH)*(p_kill_IRSpostfeed+(1-p_kill_IRSpostfeed)*p_kill_PFH))+(1-p_attraction_ADIH)*(p_block_net*((1-p_kill_ITN)*p_kill_PFH+p_kill_ITN)+(1.0f-p_block_net)*p_block_indrep*p_kill_PFH)));
        pHostNotAvailable    = (1-p_kill_PFH)*(p_block_housing+(1-p_block_housing)*(1-p_kill_IRSprefeed)*(1-p_attraction_ADIH)*(p_block_net*(1-p_kill_ITN)*(1-p_kill_PFH)+(1.0f-p_block_net)*p_block_indrep*(1.0f-p_kill_PFH)));
        pDieDuringFeeding    = (1-p_kill_PFH)*(1-p_block_housing)*(1-p_kill_IRSprefeed) *(1-p_attraction_ADIH)*(1-p_block_net)*(1-p_block_indrep)*p_dieduringfeeding;
        pDiePostFeeding      = (1-p_kill_PFH)*(1-p_block_housing)*(1-p_kill_IRSprefeed) *(1-p_attraction_ADIH) *(1-p_block_net)*(1-p_block_indrep)*(1-p_dieduringfeeding)*(p_kill_IRSpostfeed+(1-p_kill_IRSpostfeed)*p_kill_PFH);
        pSuccessfulFeedHuman = (1-p_kill_PFH)*(1-p_block_housing)*(1-p_kill_IRSprefeed) *(1-p_attraction_ADIH) *(1-p_block_net)*(1-p_block_indrep)* (1-p_dieduringfeeding)*(1-p_kill_IRSpostfeed)*(1-p_kill_PFH);
        pSuccessfulFeedAD    = (1-p_kill_PFH)*(1-p_block_housing)*(1-p_kill_IRSprefeed)*p_attraction_ADIH*(1-p_kill_ADIH)*(1-p_kill_IRSpostfeed)*(1-p_kill_PFH);

        //update intervention effect on acquisition and transmission of infection--NOTE that vector tendencies to bite an individual are already gathered into intervention_system_effects
        //gets infection for dies during feeding, dies post feeding, or successful feed, but NOT die before feeding or unable to find host
        blockIndoorVectorAcquire = pDieDuringFeeding + pDiePostFeeding + pSuccessfulFeedHuman;

        // transmission to mosquito only in case of survived feed
        blockIndoorVectorTransmit = pSuccessfulFeedHuman;

        // update probabilities for outdoor feeding outcomes
        pOutdoorDieBeforeFeeding    = 0;
        pOutdoorHostNotAvailable    = p_block_indrep;
        pOutdoorDieDuringFeeding    = ( 1.0f - p_block_indrep ) * p_dieduringfeeding;
        pOutdoorDiePostFeeding      = ( 1.0f - p_block_indrep ) * ( 1.0f - p_dieduringfeeding ) * p_kill_insecticidal_drug;
        pOutdoorSuccessfulFeedHuman = ( 1.0f - p_block_indrep ) * ( 1.0f - p_dieduringfeeding ) * ( 1.0f - p_kill_insecticidal_drug );
        blockOutdoorVectorAcquire   = pOutdoorDieDuringFeeding + pOutdoorDiePostFeeding + pOutdoorSuccessfulFeedHuman;
        blockOutdoorVectorTransmit  = pOutdoorSuccessfulFeedHuman;
    }

    int VectorInterventionsContainer::AddRef()  { return InterventionsContainer::AddRef(); }
    int VectorInterventionsContainer::Release() { return InterventionsContainer::Release(); }

    float VectorInterventionsContainer::GetDieBeforeFeeding()           { return pDieBeforeFeeding; }
    float VectorInterventionsContainer::GetHostNotAvailable()           { return pHostNotAvailable; }
    float VectorInterventionsContainer::GetDieDuringFeeding()           { return pDieDuringFeeding; }
    float VectorInterventionsContainer::GetDiePostFeeding()             { return pDiePostFeeding; }
    float VectorInterventionsContainer::GetSuccessfulFeedHuman()        { return pSuccessfulFeedHuman; }
    float VectorInterventionsContainer::GetSuccessfulFeedAD()           { return pSuccessfulFeedAD; }
    float VectorInterventionsContainer::GetOutdoorDieBeforeFeeding()    { return pOutdoorDieBeforeFeeding; }
    float VectorInterventionsContainer::GetOutdoorHostNotAvailable()    { return pOutdoorHostNotAvailable; }
    float VectorInterventionsContainer::GetOutdoorDieDuringFeeding()    { return pOutdoorDieDuringFeeding; }
    float VectorInterventionsContainer::GetOutdoorDiePostFeeding()      { return pOutdoorDiePostFeeding; }
    float VectorInterventionsContainer::GetOutdoorSuccessfulFeedHuman() { return pOutdoorSuccessfulFeedHuman; }
    float VectorInterventionsContainer::GetblockIndoorVectorAcquire()   { return blockIndoorVectorAcquire; }
    float VectorInterventionsContainer::GetblockIndoorVectorTransmit()  { return blockIndoorVectorTransmit; }
    float VectorInterventionsContainer::GetblockOutdoorVectorAcquire()  { return blockOutdoorVectorAcquire; }
    float VectorInterventionsContainer::GetblockOutdoorVectorTransmit() { return blockOutdoorVectorTransmit; }

    REGISTER_SERIALIZABLE(VectorInterventionsContainer);

    void VectorInterventionsContainer::serialize(IArchive& ar, VectorInterventionsContainer* obj)
    {
        VectorInterventionsContainer& container = *obj;
        InterventionsContainer::serialize(ar, obj);
        ar.labelElement("pDieBeforeFeeding")           & container.pDieBeforeFeeding;
        ar.labelElement("pHostNotAvailable")           & container.pHostNotAvailable;
        ar.labelElement("pDieDuringFeeding")           & container.pDieDuringFeeding;
        ar.labelElement("pDiePostFeeding")             & container.pDiePostFeeding;
        ar.labelElement("pSuccessfulFeedHuman")        & container.pSuccessfulFeedHuman;
        ar.labelElement("pSuccessfulFeedAD")           & container.pSuccessfulFeedAD;
        ar.labelElement("pOutdoorDieBeforeFeeding")    & container.pOutdoorDieBeforeFeeding;
        ar.labelElement("pOutdoorHostNotAvailable")    & container.pOutdoorHostNotAvailable;
        ar.labelElement("pOutdoorDieDuringFeeding")    & container.pOutdoorDieDuringFeeding;
        ar.labelElement("pOutdoorDiePostFeeding")      & container.pOutdoorDiePostFeeding;
        ar.labelElement("pOutdoorSuccessfulFeedHuman") & container.pOutdoorSuccessfulFeedHuman;
        ar.labelElement("blockIndoorVectorAcquire")    & container.blockIndoorVectorAcquire;
        ar.labelElement("blockIndoorVectorTransmit")   & container.blockIndoorVectorTransmit;
        ar.labelElement("blockOutdoorVectorAcquire")   & container.blockOutdoorVectorAcquire;
        ar.labelElement("blockOutdoorVectorTransmit")  & container.blockOutdoorVectorTransmit;
    }
}
