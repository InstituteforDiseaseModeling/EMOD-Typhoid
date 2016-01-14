/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <math.h>
#include "Exceptions.h"
#include "SimulationEnums.h" // to get DistributionFunction enum. Don't want utils reaching into Eradication though. TBD!!!

struct Gamma
{
    inline static double WindschitlApproximation ( double z )
    {
        if (z < 0)
        {
            //LOG_WARN_F("GammaApprox(z) is a terrible approximation with z<0 (z=%f).\n",z);
            return 0;
        }
        if (z == 0)
        {
            //LOG_WARN_F("GammaApprox(z) called with z=0.  Returning DBL_MAX instead of infinity.\n");
            return FLT_MAX;
        }
        return (2.5066282746310002 * sqrt(1.0/z) * pow((z/2.718281828459045) * sqrt(z*sinh(1/z) + 1/(810*pow(z,6))), z));
    }
};

namespace Kernel {

class Probability
{
    public:
        static Probability * getInstance()
        {
            if( _instance == nullptr )
            {
                _instance = new Probability();
            }
            return _instance;
        }

        double Probability::fromDistribution(Kernel::DistributionFunction::Enum distribution_flag, double param1, double param2 = 0.0, double default_value = 0.0);

    protected:

        Probability()
        {
        }

        static Probability * _instance;
};

#define LOG_2 0.6931472f
}
