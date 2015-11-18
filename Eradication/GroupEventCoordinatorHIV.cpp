/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

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
#include "Individual.h"
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
        initConfig("Target_Gender", target_gender, inputJson, MetadataDescriptor::Enum("target_gender", Target_Gender_DESC_TEXT, MDD_ENUM_ARGS(TargetGender)));
        initConfigTypeMap("Target_Age_Min", &target_age_min, Target_Age_Min_DESC_TEXT, 0.0f, 200.0f, 0 ); // 
        initConfigTypeMap("Target_Age_Max", &target_age_max, Target_Age_Max_DESC_TEXT, 0.0f, 200.0f, 200.0f);
        initConfigTypeMap("Time_Offset", &time_offset, Time_Offset_DESC_TEXT, 0.0f, FLT_MAX, 0.0f);
        //initConfigTypeMap("Property_Restrictions", &property_restrictions, Property_Restriction_DESC_TEXT );
        
        bool retValue = StandardInterventionDistributionEventCoordinator::Configure(inputJson);
        return retValue;
    }

    // ctor
    GroupInterventionDistributionEventCoordinatorHIV::GroupInterventionDistributionEventCoordinatorHIV()
    {
        LOG_DEBUG("GroupInterventionDistributionEventCoordinatorHIV ctor\n"); 
    } 
   
    bool
    GroupInterventionDistributionEventCoordinatorHIV::qualifiesByGroup(
        const IIndividualHumanEventContext * const pIndividual
    )
    const
    {
        bool retQualifies = true;


        IIndividualHumanCoinfection* Coinf_ind = nullptr;
        if(const_cast<IIndividualHumanEventContext*>(pIndividual)->QueryInterface( GET_IID( IIndividualHumanCoinfection ), (void**)&Coinf_ind ) != s_OK)
        { //error here
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pIndividual", "IIndividualHumanCoinfection", "IIndividualHumanEventContext" );
        }

    

        if (pIndividual->GetAge() < target_age_min * DAYSPERYEAR || pIndividual->GetAge() > target_age_max * DAYSPERYEAR) {return false; }
        
        

        if (pIndividual->GetGender() == Gender::FEMALE && target_gender == TargetGender::Male ) {return false;}

        if (pIndividual->GetGender() == Gender::MALE && target_gender == TargetGender::Female) {return false;}
        

        if (target_group == TargetGroupType::HIVNegative )
        {    
            if (Coinf_ind->HasHIV() ) {return false;}
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

            IIndividualHumanContext* pIndHu;

            if (const_cast<IIndividualHumanEventContext*>(ihec)->QueryInterface( GET_IID( IIndividualHumanContext ), (void**)&pIndHu ) != s_OK)
        { //error here
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pIndividual", "IIndividualHumanCoinfection", "IIndividualHumanEventContext" );
        }
            
            demographic_coverage = pIndHu->GetDemographicsDistribution(NodeDemographicsDistribution::HIVCoinfectionDistribution)->DrawResultValue(ihec->GetGender() == Gender::FEMALE, (parent->GetSimulationTime().time - time_offset), ihec->GetAge() );
        
        
        
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
