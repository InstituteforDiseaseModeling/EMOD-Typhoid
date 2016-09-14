/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

namespace Kernel
{
    struct TBHIVParameters
    {
        int num_cd4_time_steps;
        float cd4_time_step;
        bool coinfection_incidence;
        bool enable_coinfection_mortality;

        TBHIVParameters()
        : num_cd4_time_steps(0)
        , cd4_time_step(0.0)
        , coinfection_incidence(false)
        , enable_coinfection_mortality(false)
        {
        }
    };
}
