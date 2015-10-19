/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "IdmApi.h"
#include "SimulationEnums.h"
#include <map>
#include <vector>

using namespace std;

namespace Kernel {

    struct IDMAPI IPairFormationStats {

        virtual void ResetEligible() = 0;
        virtual void UpdateEligible(float age_in_days, int sex, RiskGroup::Enum risk_group, int delta) = 0;
        virtual const map<int, vector<int>>& GetEligible(RiskGroup::Enum risk_group) = 0;

        virtual ~IPairFormationStats() {}
    };
}