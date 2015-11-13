/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "VectorContexts.h"
#include "Infection.h"

namespace Kernel
{
    class InfectionVector : public Infection
    {
    public:
        static InfectionVector *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~InfectionVector(void);

    protected:
        InfectionVector();
        InfectionVector(IIndividualHumanContext *context);
        /* clorton virtual */ void Initialize(suids::suid _suid) /* clorton override */;

        DECLARE_SERIALIZABLE(InfectionVector);

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, InfectionVector& inf, unsigned int  file_version );
#endif
    };
}
