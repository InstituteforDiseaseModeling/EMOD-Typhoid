/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "AntiPoliovirusDrug.h"

#include "Exceptions.h"
#include "PolioInterventionsContainer.h"    // for IPolioDrugEffectsApply methods
#include "SimulationConfig.h"               // for global-context access to PKPDmodel (!)
#include "RANDOM.h"

static const char * _module = "AntipoliovirusDrug";

namespace Kernel
{

    BEGIN_QUERY_INTERFACE_DERIVED(AntipoliovirusDrug, GenericDrug)
        //HANDLE_INTERFACE(IPolioDrugEffects)
    END_QUERY_INTERFACE_DERIVED(AntipoliovirusDrug, GenericDrug)
    IMPLEMENT_FACTORY_REGISTERED(AntipoliovirusDrug)

    AntipoliovirusDrug::~AntipoliovirusDrug()
    { }

    AntipoliovirusDrug::AntipoliovirusDrug()
        : GenericDrug(),
        adherence_rate(1.0f),
        titer_efficacy(1.0f),
        infection_duration_efficacy(1.0f),
        response_probability(1.0f),
        ipda(nullptr)
    {
        initSimTypes( 1, "POLIO_SIM" );
    }

    bool
    AntipoliovirusDrug::Configure(
        const Configuration * inputJson
    )
    {
        initConfigTypeMap( "Adherence_Rate", &adherence_rate, DRUG_Adherence_Rate_DESC_TEXT, 0.0f, 1.0f, 1.0f);
        initConfigTypeMap( "Titer_Efficacy", &titer_efficacy, DRUG_Titer_Efficacy_DESC_TEXT, 0.0f, 1.0f, 1.0f);
        initConfigTypeMap( "Infection_Duration_Efficacy", &infection_duration_efficacy, DRUG_Infection_Duration_Efficacy_DESC_TEXT, 0.0f, 1.0f, 1.0f);
        initConfigTypeMap( "Responder_Rate", &response_probability, DRUG_Repsonder_Rate_DESC_TEXT, 0.0f, 1.0f, 1.0f);

        return GenericDrug::Configure( inputJson );
    }

    bool
    AntipoliovirusDrug::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * pCCO
    )
    {
        if (s_OK != context->QueryInterface(GET_IID(IPolioDrugEffectsApply), (void**)&ipda) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IPolioDrugEffectsApply", "IIndividualHumanInterventionsContext" );
        }

        // just add in another Drug to list, can later check the person's records and apply accordingly (TODO)
        return GenericDrug::Distribute( context, pCCO );
    }

    void AntipoliovirusDrug::ResetForNextDose(float dt)
    {
        // reset dosing timer and decrement remaining doses
        GenericDrug::ResetForNextDose(dt);

        // check adherence for dropout after each dose
        if(randgen->e() < (1.0f - adherence_rate))
        {
            LOG_DEBUG_F("Nonadherence, p= %f", 1.0f - adherence_rate);
            remaining_doses = 0;
        }
    }

    void
    AntipoliovirusDrug::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        LOG_DEBUG("setting drug context...");

        if (s_OK != context->GetInterventionsContext()->QueryInterface(GET_IID(IPolioDrugEffectsApply), (void**)&ipda) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context->GetInterventionsContext()", "IPolioDrugEffectsApply", "IIndividualHumanInterventionsContext" );
        }

        return GenericDrug::SetContextTo( context );
    }

    void AntipoliovirusDrug::ConfigureDrugTreatment()
    {
        LOG_DEBUG("Called ConfigureDrugTreatment...");
        LOG_DEBUG_F("remaining= %d, response= %f ...\n", remaining_doses, response_probability);

        // modify results
        if(randgen->e() > response_probability) // is this individual a responder to the drug? if not, do no drug actions
        {
            LOG_DEBUG(" NonResponder... ");
            remaining_doses = 0;
        }
    }

    void AntipoliovirusDrug::ApplyEffects()
    {
        assert(ipda);
        ipda->ApplyDrugVaccineReducedAcquireEffect( GetDrugReducedAcquire() );
        ipda->ApplyDrugVaccineReducedTransmitEffect( GetDrugReducedTransmit() );

        ipda->ApplyDrugTiterEffect( titer_efficacy * current_efficacy );
        ipda->ApplyDrugDurationEffect( infection_duration_efficacy * current_efficacy );

        LOG_DEBUG_F("antipoliovirus Apply Effects called with efficacy %f\n.", current_efficacy);
    }
}

// TODO: move to single serialization block
#if 0
namespace Kernel {
    template <typename Archive>
    void serialize(Archive &ar, AntipoliovirusDrug& drug, const unsigned int v)
    {
        ar & drug.adherence_rate;
        ar & drug.titer_efficacy;
        ar & drug.infection_duration_efficacy;

        ar & boost::serialization::base_object<GenericDrug>(drug);
    }
}
#endif
