/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "MigrateIndividuals.h"
#include "IMigrate.h"

#include "NodeEventContext.h"  // for INodeEventContext (ICampaignCostObserver)

static const char * _module = "MigrateIndividuals";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(MigrateIndividuals)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
    END_QUERY_INTERFACE_BODY(MigrateIndividuals)

    IMPLEMENT_FACTORY_REGISTERED(MigrateIndividuals)

    bool MigrateIndividuals::Configure(
        const Configuration * inputJson
    )
    {
        initConfigTypeMap( "NodeID_To_Migrate_To", &destination_external_node_id, "TBD", 0, INT_MAX, 0 );
        initConfigTypeMap( "Is_Family_Trip", &is_family_trip, "TBD", false );
        initConfigTypeMap( "Is_Moving", &is_moving, "TBD", false );

        duration_before_leaving.Configure( this, inputJson );
        duration_at_node.Configure( this, inputJson );

        bool ret = JsonConfigurable::Configure( inputJson );

        if( ret )
        {
            duration_before_leaving.CheckConfiguration();
            duration_at_node.CheckConfiguration();
        }
        return ret;
    }

    MigrateIndividuals::MigrateIndividuals()
        : BaseIntervention()
        , parent( nullptr )
        , destination_external_node_id( 0 )
        , duration_before_leaving()
        , duration_at_node()
        , is_family_trip( false )
        , is_moving( false )
    {
        duration_before_leaving.SetTypeNameDesc( "Duration_Before_Leaving_Distribution_Type", "TBD" );
        duration_before_leaving.AddSupportedType( DistributionFunction::FIXED_DURATION,       "Duration_Before_Leaving_Fixed",              "TBD", "", "" );
        duration_before_leaving.AddSupportedType( DistributionFunction::UNIFORM_DURATION,     "Duration_Before_Leaving_Uniform_Min",        "TBD", "Duration_Before_Leaving_Uniform_Max", "TBD" );
        duration_before_leaving.AddSupportedType( DistributionFunction::GAUSSIAN_DURATION,    "Duration_Before_Leaving_Gausian_Mean",       "TBD", "Duration_Before_Leaving_Gausian_StdDev", "TBD" );
        duration_before_leaving.AddSupportedType( DistributionFunction::EXPONENTIAL_DURATION, "Duration_Before_Leaving_Exponential_Period", "TBD", "", "" );
        duration_before_leaving.AddSupportedType( DistributionFunction::POISSON_DURATION,     "Duration_Before_Leaving_Poisson_Mean",       "TBD", "", "" );

        duration_at_node.SetTypeNameDesc( "Duration_At_Node_Distribution_Type", "TBD" );
        duration_at_node.AddSupportedType( DistributionFunction::FIXED_DURATION,       "Duration_At_Node_Fixed",              "TBD", "", "" );
        duration_at_node.AddSupportedType( DistributionFunction::UNIFORM_DURATION,     "Duration_At_Node_Uniform_Min",        "TBD", "Duration_At_Node_Uniform_Max", "TBD" );
        duration_at_node.AddSupportedType( DistributionFunction::GAUSSIAN_DURATION,    "Duration_At_Node_Gausian_Mean",       "TBD", "Duration_At_Node_Gausian_StdDev", "TBD" );
        duration_at_node.AddSupportedType( DistributionFunction::EXPONENTIAL_DURATION, "Duration_At_Node_Exponential_Period", "TBD", "", "" );
        duration_at_node.AddSupportedType( DistributionFunction::POISSON_DURATION,     "Duration_At_Node_Poisson_Mean",       "TBD", "", "" );
    }

    MigrateIndividuals::MigrateIndividuals( const MigrateIndividuals& master )
        : BaseIntervention( master )
        , parent( nullptr )
        , destination_external_node_id( master.destination_external_node_id )
        , duration_before_leaving( master.duration_before_leaving )
        , duration_at_node( master.duration_at_node )
        , is_family_trip( master.is_family_trip )
        , is_moving( master.is_moving )
    {
    }

    void MigrateIndividuals::Update( float dt )
    {
        // expire the intervention
        expired = true;

        INodeContext* p_node_context = parent->GetEventContext()->GetNodeEventContext()->GetNodeContext();

        suids::suid destination_id = p_node_context->GetParent()->GetNodeSuid( destination_external_node_id );

        float duration_before = duration_before_leaving.CalculateDuration();
        float duration_at = duration_at_node.CalculateDuration();

        if( is_family_trip )
        {
            p_node_context->SetWaitingForFamilyTrip( destination_id, MigrationType::SEA_MIGRATION, duration_before, duration_at );
        }
        else
        {
            IMigrate * im = NULL;
            if (s_OK != parent->QueryInterface(GET_IID(IMigrate), (void**)&im) )
            {
                throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "parent", "IMigrate", "IIndividualHumanContext");
            }
            im->SetMigrating( destination_id, MigrationType::SEA_MIGRATION, duration_before, duration_at, is_moving );
        }
    }
    REGISTER_SERIALIZABLE(MigrateIndividuals);

    void MigrateIndividuals::serialize(IArchive& ar, MigrateIndividuals* obj)
    {
        BaseIntervention::serialize( ar, obj );
        MigrateIndividuals& mt = *obj;
        ar.labelElement("destination_external_node_id") & mt.destination_external_node_id;
        ar.labelElement("duration_before_leaving"     ) & mt.duration_before_leaving;
        ar.labelElement("duration_at_node"            ) & mt.duration_at_node;
        ar.labelElement("is_family_trip"              ) & mt.is_family_trip;
        ar.labelElement("is_moving"                   ) & mt.is_moving;
    }
}
