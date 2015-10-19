/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Susceptibility.h"

namespace Kernel
{
    class SusceptibilityEnvironmental : public Susceptibility
    {
    public:
        static SusceptibilityEnvironmental *CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod);
        virtual ~SusceptibilityEnvironmental(void);

    protected:
        float demographic_risk;

        SusceptibilityEnvironmental();
        SusceptibilityEnvironmental(IIndividualHumanContext *context);
        void Initialize(float age, float immmod, float riskmod);

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, SusceptibilityEnvironmental& sus, const unsigned int file_version );
#endif
    };
}
