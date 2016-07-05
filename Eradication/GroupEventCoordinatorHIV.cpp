/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include <sstream>
#include "Sugar.h"
#include "Environment.h"
#include "GroupEventCoordinatorHIV.h"
#include "Configuration.h"
#include "ConfigurationImpl.h"
#include "FactorySupport.h"
#include "InterventionFactory.h"
#include "IIndividualHuman.h"
#include "NodeDemographics.h"
#include "NodeEventContext.h"
#include "Log.h"
#include "TBContexts.h"
#include "IndividualCoinfection.h"
#include "Properties.h"

static const char * _module = "GroupEventCoordinatorHIV";


namespace Kernel
{

    IMPLEMENT_FACTORY_REGISTERED(GroupInterventionDistributionEventCoordinatorHIV)

    IMPL_QUERY_INTERFACE2(GroupInterventionDistributionEventCoordinatorHIV, IEventCoordinator, IConfigurable)

      QuickBuilder
    GroupInterventionDistributionEventCoordinatorHIV::GetSchema()
    {
        return QuickBuilder( jsonSchemaBase );
    }

    bool
    GroupInterventionDistributionEventCoordinatorHIV::Configure( const Configuration * inputJson)
    {
        initConfig("Target_Group", target_group, inputJson, MetadataDescriptor::Enum("target_group", Target_Group_DESC_TEXT, MDD_ENUM_ARGS(TargetGroupType))) ;
        initConfigTypeMap("Time_Offset", &time_offset, Time_Offset_DESC_TEXT, 0.0f, FLT_MAX, 0.0f);
        
        bool retValue = StandardInterventionDistributionEventCoordinator::Configure(inputJson);
        return retValue;
    }

    // ctor
    GroupInterventionDistributionEventCoordinatorHIV::GroupInterventionDistributionEventCoordinatorHIV()
        : StandardInterventionDistributionEventCoordinator()
        , target_group(TargetGroupType::Everyone)
        , time_offset(0.0f)
    {
        LOG_DEBUG("GroupInterventionDistributionEventCoordinatorHIV ctor\n"); 
    } 
   
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !!! 1/11/2016 DMB
    // !!! This method does not get called.  The class is not a subclass of 
    // !!! GroupInterventionDistributionEventCoordinator so it is not being
    // !!! restricted by the restrictions there.  We should consider whether
    // !!! we want to extend DemographicRestrictions.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    bool
    GroupInterventionDistributionEventCoordinatorHIV::qualifiesByGroup(
        const IIndividualHumanEventContext * const pIndividual
    )
    const
    {
        bool retQualifies = true;

        if( (pIndividual->GetAge() < demographic_restrictions.GetMinimumAge() * DAYSPERYEAR) || 
            (pIndividual->GetAge() > demographic_restrictions.GetMaximumAge() * DAYSPERYEAR) ) 
        {
            return false;
        }

        if( (pIndividual->GetGender()                        == Gender::FEMALE    ) &&
            (demographic_restrictions.GetTargetDemographic() == TargetGender::Male) ) 
        {
            return false;
        }

        if( (pIndividual->GetGender()                        == Gender::MALE        ) &&
            (demographic_restrictions.GetTargetDemographic() == TargetGender::Female) ) 
        {
            return false;
        }

        if( target_group == TargetGroupType::HIVNegative )
        {    
            IIndividualHumanCoinfection* Coinf_ind = NULL;
            if(const_cast<IIndividualHumanEventContext*>(pIndividual)->QueryInterface( GET_IID( IIndividualHumanCoinfection ), (void**)&Coinf_ind ) != s_OK)
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pIndividual", "IIndividualHumanCoinfection", "IIndividualHumanEventContext" );
            }

            if( Coinf_ind->HasHIV() )
            {
                return false;
            }
        }
        
        //also check the base class
        //retQualifies = StandardInterventionDistributionEventCoordinator::qualifiesDemographically( pIndividual );

        return retQualifies;
    }

   bool
    GroupInterventionDistributionEventCoordinatorHIV::visitIndividualCallback( 
        IIndividualHumanEventContext *ihec,
        float & incrementalCostOut,
        ICampaignCostObserver * pICCO
    )
   {
        bool retValue = true;

        IIndividualHumanContext* pIndHu = nullptr;

        if (const_cast<IIndividualHumanEventContext*>(ihec)->QueryInterface( GET_IID( IIndividualHumanContext ), (void**)&pIndHu ) != s_OK)
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pIndividual", "IIndividualHumanCoinfection", "IIndividualHumanEventContext" );
        }
        
        float dc = pIndHu->GetDemographicsDistribution(NodeDemographicsDistribution::HIVCoinfectionDistribution)->DrawResultValue( ihec->GetGender() == Gender::FEMALE, (parent->GetSimulationTime().time - time_offset), ihec->GetAge() );
        demographic_restrictions.SetDemographicCoverage( dc );
        
        retValue = StandardInterventionDistributionEventCoordinator::visitIndividualCallback( ihec, incrementalCostOut, pICCO);
        
        return retValue;
    }
}
 

#if 0
namespace Kernel
{
    template<class Archive>
    void serialize(Archive &ar, GroupInterventionDistributionEventCoordinatorHIV &ec, const unsigned int v)
    {
        ar & ec.target_disease_state;

        ar & ec.node_suids;

        ar & boost::serialization::base_object<StandardInterventionDistributionEventCoordinator>(ec);
    }
}
#endif
