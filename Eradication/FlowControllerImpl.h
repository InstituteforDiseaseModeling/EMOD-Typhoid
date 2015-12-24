/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "IPairFormationFlowController.h"
#include "IPairFormationAgent.h"
#include "IPairFormationStats.h"
#include "IPairFormationRateTable.h"
#include "IPairFormationParameters.h"

#include <map>
#include <vector>

namespace Kernel 
{

    class IDMAPI FlowControllerImpl : public IPairFormationFlowController 
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE();
    public:

        virtual void UpdateEntryRates();

        static IPairFormationFlowController* CreateController(
            IPairFormationAgent*,
            IPairFormationStats*,
            IPairFormationRateTable*,
            const IPairFormationParameters*);

    protected:
        FlowControllerImpl( IPairFormationAgent* agent=nullptr,
                            IPairFormationStats* stats=nullptr, 
                            IPairFormationRateTable* table=nullptr,
                            const IPairFormationParameters* params=nullptr );
        virtual ~FlowControllerImpl();

        void UpdateDesiredFlow();

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        IPairFormationAgent* pair_formation_agent;
        IPairFormationStats* pair_formation_stats;
        IPairFormationRateTable* rate_table;
        const IPairFormationParameters* parameters;

        float rate_ratio[Gender::COUNT];

        map<int, vector<float>> desired_flow;

        float base_pair_formation_rate;

        DECLARE_SERIALIZABLE(FlowControllerImpl);
#pragma warning( pop )
    };
}
