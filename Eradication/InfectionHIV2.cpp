/*****************************************************************************

Copyright (c) 2014 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#include <stdafx.h>
#include "InfectionHIV2.h"

static const char * _module = "InfectionHIV2";

namespace Kernel {

    //
    // InfectionHIV2 (trivial drop-in replacement)
    //
    //
    // Ctors and initializers common to all Infection types
    Infection *
    InfectionHIV2::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        InfectionHIV2 *newinfection = _new_ InfectionHIV2(context);
        newinfection->Initialize(_suid);
        return newinfection;
    }

    InfectionHIV2::~InfectionHIV2(void) { }
    InfectionHIV2::InfectionHIV2()
        : hiv_parent( NULL )
        , viral_suppression_active( false )
        //, ViralLoad(0)
    {
    }

    InfectionHIV2::InfectionHIV2(IIndividualHumanContext *context)
        : InfectionSTI(context)
        , hiv_parent( NULL )
        , viral_suppression_active( false )
    {
    }

    BEGIN_QUERY_INTERFACE_BODY(InfectionHIV2)
        HANDLE_INTERFACE(IInfectionHIV)
    END_QUERY_INTERFACE_BODY(InfectionHIV2)

    void InfectionHIV2::Initialize(suids::suid _suid)
    {
        InfectionSTI::Initialize(_suid);
        if( s_OK != parent->QueryInterface(GET_IID(IIndividualHumanHIV), (void**)&hiv_parent) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHumanHIV", "IndividualHuman" );
        }
        _viralLoadTimer = Probability::getInstance()->fromDistribution( DistributionFunction::EXPONENTIAL_DURATION, 0.001, 0 );
        // decide how long they stay in acute
        _hivStageTimer = Probability::getInstance()->fromDistribution( DistributionFunction::EXPONENTIAL_DURATION, 0.001, 0 );
        m_infection_stage = HIVInfectionStage::ACUTE;
        infectious_timer = MAX_HUMAN_AGE*DAYSPERYEAR;
        total_duration = infectious_timer;
        duration = 0.0f;
        LOG_DEBUG_F( "Individual %lu initialized HIV infection with hivStageTimer value of %f.\n", parent->GetSuid().data, _hivStageTimer );
    } 

    void InfectionHIV2::SetParameters(StrainIdentity* infstrain, int incubation_period_override)
    {
        //InfectionSTI::SetParameters( infstrain, incubation_period_override );
    }

    void InfectionHIV2::Update(float dt, Susceptibility* immunity)
    {
        InfectionSTI::Update(dt, immunity);
        if( viral_suppression_active == false )
        {
            return;
        }

        _viralLoadTimer -= dt;
        _hivStageTimer -= dt;
        if( _hivStageTimer <= 0 )
        {
            // progress viral load stage
            switch( GetStage() )
            {
                case HIVInfectionStage::ACUTE:
                    m_infection_stage = HIVInfectionStage::LATENT;
                    /// determine how long they stay in latent
                    _hivStageTimer = Probability::getInstance()->fromDistribution( DistributionFunction::EXPONENTIAL_DURATION, 0.001, 0 );
                break;

                case HIVInfectionStage::LATENT:
                    m_infection_stage = HIVInfectionStage::AIDS;
                    /// determine how long they stay in AIDS
                    _hivStageTimer = Probability::getInstance()->fromDistribution( DistributionFunction::EXPONENTIAL_DURATION, 0.001, 0 );
                break;

                case HIVInfectionStage::AIDS:
                    // death
                    StateChange = InfectionStateChange::Fatal;
                break;
            }
        }
        LOG_DEBUG_F( "Individual %lu is in %s stage of HIV.\n", parent->GetSuid().data, HIVInfectionStage::pairs::lookup_key( m_infection_stage ) );
    }

    //
    // Initializers specific to STI and HIV...
    //
    // This is an initialization function that gets called at disease onset
    /*void
    InfectionHIV2::SetupNonSuppressedDiseaseTimers()
    {
    }*/

    // 
    // Getters
    // 
    float
    InfectionHIV2::GetInfectiousness() const
    {
        // Almost certainly want an infectivity multiplier by stage
        float retInf = InfectionSTI::GetInfectiousness();
        return retInf;
    }

    NaturalNumber InfectionHIV2::GetViralLoad()
    const
    {
        // return it if we're modeling it.
        return 0;
    }

    float
    InfectionHIV2::GetPrognosis()
    const
    {
        // Only have this if we precalculate their death
        return 1.0f;
    }

    const HIVInfectionStage::Enum&
    InfectionHIV2::GetStage()
    const
    {
        return m_infection_stage;
    }

    // Do we really need this for the alternate model?
    // This is used by some diagnostics, some reporters, and internally by the default InfectionHIV for 
    // calculating ComputeDurationFromEnrollmentToArtAidsDeath.
    float
    InfectionHIV2::GetWHOStage()
    const
    {
        return 1.0f;
    }

    //
    // Intervention-related
    // 
    /*bool InfectionHIV2::ApplyDrugEffects(float dt, Susceptibility* immunity)
    {
        // Check for valid inputs
    }*/

    // This is an initialization function that gets called at intervention onset
    // Implementation is entrely dependent on whether or not this intra-host infection
    // model needs to do something with an ART-like intervention.
    void
    InfectionHIV2::SetupSuppressedDiseaseTimers()
    { 
        viral_suppression_active = true;
    }

    void InfectionHIV2::ApplySuppressionDropout()
    {
        viral_suppression_active = false;
    }

    void InfectionHIV2::ApplySuppressionFailure()
    {
        viral_suppression_active = false;
    }
}
