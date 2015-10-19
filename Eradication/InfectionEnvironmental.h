/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Contexts.h"
#include "Infection.h"

namespace Kernel
{
    class InfectionEnvironmental : public Infection
    {
    public:
        static InfectionEnvironmental *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~InfectionEnvironmental(void);

        virtual void Update(float dt, Susceptibility* immunity = NULL);
        virtual void SetParameters(StrainIdentity* _infstrain=NULL, int incubation_period_override = -1 );

    protected:
        InfectionEnvironmental(IIndividualHumanContext *context);
        void Initialize(suids::suid _suid);
        InfectionEnvironmental();

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;

        template<class Archive>
        friend void serialize(Archive & ar, InfectionEnvironmental& inf, const unsigned int file_version );
#endif    
    };
}
