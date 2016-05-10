/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "RevaccinatableVaccine.h"
#include "Interventions.h"
#include "IndividualEventContext.h"
#include "NodeEventContext.h"

static const char* _module = "RevaccinatableVaccine";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(RevaccinatableVaccine,SimpleVaccine)
    END_QUERY_INTERFACE_DERIVED(RevaccinatableVaccine,SimpleVaccine)

    IMPLEMENT_FACTORY_REGISTERED(RevaccinatableVaccine)

    bool
    RevaccinatableVaccine::Configure(
        const Configuration * inputJson
    )
    {
        initConfigTypeMap( "Distributed_Event_Trigger",             &m_DistributedEventTrigger,           RV_Distributed_Event_Trigger_DESC_TEXT,             NO_TRIGGER_STR );
        initConfigTypeMap( "Expired_Event_Trigger",                 &m_ExpiredEventTrigger,               RV_Expired_Event_Trigger_DESC_TEXT,                 NO_TRIGGER_STR );
        initConfigTypeMap( "Duration_To_Wait_Before_Revaccination", &m_DurationToWaitBeforeRevaccination, RV_Duration_To_Wait_Before_Revaccination_DESC_TEXT, 0, FLT_MAX, FLT_MAX);

        bool configured = SimpleVaccine::Configure( inputJson );
        return configured;
    }

    RevaccinatableVaccine::RevaccinatableVaccine() 
    : SimpleVaccine()
    , m_DurationToWaitBeforeRevaccination(FLT_MAX)
    , m_TimeSinceVaccination(0.0)
    , m_DistributedEventTrigger(NO_TRIGGER_STR)
    , m_ExpiredEventTrigger(NO_TRIGGER_STR)
    {
    }

    RevaccinatableVaccine::RevaccinatableVaccine( const RevaccinatableVaccine& master )
    : SimpleVaccine( master )
    , m_DurationToWaitBeforeRevaccination( master.m_DurationToWaitBeforeRevaccination )
    , m_TimeSinceVaccination(              master.m_TimeSinceVaccination              )
    , m_DistributedEventTrigger(           master.m_DistributedEventTrigger           )
    , m_ExpiredEventTrigger(               master.m_ExpiredEventTrigger               )
    {
    }

    RevaccinatableVaccine::~RevaccinatableVaccine()
    {
    }

    bool RevaccinatableVaccine::Distribute( IIndividualHumanInterventionsContext *context, ICampaignCostObserver * pCCO )
    {
        // ------------------------------------------------------------------------------------------------
        // --- Check if the person is already vaccinated.  If they are see if we should revaccinate them.
        // --- For example, if the vaccine becomes ineffective after two years, you might want to get
        // --- re-vaccinated at 18-months to ensure you have good coverage.
        // ------------------------------------------------------------------------------------------------
        bool distribute = true;
        auto existing_vaccine_list = context->GetInterventionsByType(typeid(*this).name());
        for( auto p_di : existing_vaccine_list )
        {
            RevaccinatableVaccine* p_existing_vaccine = dynamic_cast<RevaccinatableVaccine*>(p_di);
            if( !p_existing_vaccine->AllowRevaccination() )
            {
                distribute = false;
                break;
            }
        }

        if( distribute )
        {
            distribute = SimpleVaccine::Distribute( context, pCCO );
        }

        if( distribute && (m_DistributedEventTrigger != NO_TRIGGER_STR) )
        {
            INodeTriggeredInterventionConsumer* broadcaster = nullptr;
            if (s_OK != context->GetParent()->GetEventContext()->GetNodeEventContext()->QueryInterface(GET_IID(INodeTriggeredInterventionConsumer), (void**)&broadcaster))
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__,
                    "context->GetParent()->GetEventContext()->GetNodeEventContext()", 
                    "INodeTriggeredInterventionConsumer",
                    "INodeEventContext" );
            }
            broadcaster->TriggerNodeEventObserversByString( context->GetParent()->GetEventContext(), m_DistributedEventTrigger );
        }

        return distribute;
    }

    void RevaccinatableVaccine::Update( float dt )
    {
        SimpleVaccine::Update( dt );
        m_TimeSinceVaccination += dt;

        if( expired && (m_ExpiredEventTrigger != NO_TRIGGER_STR) )
        {
            INodeTriggeredInterventionConsumer* broadcaster = nullptr;
            if (s_OK != parent->GetEventContext()->GetNodeEventContext()->QueryInterface(GET_IID(INodeTriggeredInterventionConsumer), (void**)&broadcaster))
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__,
                    "parent->GetEventContext()->GetNodeEventContext()", 
                    "INodeTriggeredInterventionConsumer",
                    "INodeEventContext" );
            }
            broadcaster->TriggerNodeEventObserversByString( parent->GetEventContext(), m_ExpiredEventTrigger );
        }
    }

    bool RevaccinatableVaccine::AllowRevaccination() const
    {
        bool allow = false;
        if( m_TimeSinceVaccination >= m_DurationToWaitBeforeRevaccination )
        {
            allow = true;
        }
        return allow;
    }

    REGISTER_SERIALIZABLE(RevaccinatableVaccine);

    void RevaccinatableVaccine::serialize(IArchive& ar, RevaccinatableVaccine* obj)
    {
        SimpleVaccine::serialize( ar, obj );
        RevaccinatableVaccine& vaccine = *obj;
        ar.labelElement( "m_TimeSinceVaccination"              ) & vaccine.m_TimeSinceVaccination;
        ar.labelElement( "m_DurationToWaitBeforeRevaccination" ) & vaccine.m_DurationToWaitBeforeRevaccination;
        ar.labelElement( "m_DistributedEventTrigger"           ) & vaccine.m_DistributedEventTrigger;
        ar.labelElement( "m_ExpiredEventTrigger"               ) & vaccine.m_ExpiredEventTrigger;
    }
}
