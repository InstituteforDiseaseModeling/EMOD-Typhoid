/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "BoostLibWrapper.h"

#include "EventCoordinator.h"
#include "Configure.h"
#include "NodeEventContext.h"
#include "StandardEventCoordinator.h"
#include "NodeEventContext.h"
#include "Node.h"

namespace Kernel
{
    //////////////////////////////////////////////////////////////////////////
    // Example implementations

    struct ICampaignCostObserver; // nasty fwd declaration coz I'm scared to include NodeEventContext.h! :)
    // Standard distribution ec that just gives out the intervention once to the fraction of people specified by the coverage parameter
        // Triggered distribution ec gives out the intervention once to the fraction of people specified by the coverage parameter
    // This ec is triggered in the sense that the intervention occurs delay_days_after_trigger after inset chart channel channel_name
    // is greater than or equal to threshold_value

    // Gives the intervention within a specific group only (ie TBpositives, thus avoiding delivery of the intervention to people who are ineligible) 
    // Within that group, only delivers to a fraction of people (specified by the coverage parameter)
    class GroupInterventionDistributionEventCoordinatorHIV : public StandardInterventionDistributionEventCoordinator
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(EventCoordinatorFactory, GroupInterventionDistributionEventCoordinatorHIV, IEventCoordinator)    
    public:
        DECLARE_CONFIGURED(GroupInterventionDistributionEventCoordinatorHIV)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    public:
        GroupInterventionDistributionEventCoordinatorHIV();
        // IEventCoordinator

         virtual bool visitIndividualCallback(IIndividualHumanEventContext *ihec, float &incrementalCostOut, ICampaignCostObserver * pICCO );

    protected:
  
    virtual bool qualifiesByGroup( const IIndividualHumanEventContext * pIndividual ) const;    

    TargetGender::Enum target_gender;
    TargetGroupType::Enum target_group;
    
    float time_offset; //time used in demographic file matrix is simulation_time - time_offset
    };
}
