/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "SusceptibilityEnvironmental.h"

namespace Kernel
{
    SusceptibilityEnvironmental::SusceptibilityEnvironmental() : Susceptibility()
    {
    }

    SusceptibilityEnvironmental::SusceptibilityEnvironmental(IIndividualHumanContext *context) : Susceptibility(context)
    {
    }

    void SusceptibilityEnvironmental::Initialize(float _age, float _immmod, float _riskmod)
    {
        Susceptibility::Initialize(_age, _immmod, _riskmod);

        demographic_risk = _riskmod; 
    }

    SusceptibilityEnvironmental *SusceptibilityEnvironmental::CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod)
    {
        SusceptibilityEnvironmental *newsusceptibility = _new_ SusceptibilityEnvironmental(context);
        newsusceptibility->Initialize(age, immmod, riskmod);

        return newsusceptibility;
    }

    SusceptibilityEnvironmental::~SusceptibilityEnvironmental(void)
    {
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::SusceptibilityEnvironmental)
namespace Kernel {
    template<class Archive>
    void serialize(Archive & ar, SusceptibilityEnvironmental& sus, const unsigned int file_version )
    {
        ar & sus.demographic_risk;
        ar & boost::serialization::base_object<Susceptibility>(sus);
    }
    template void serialize( boost::archive::binary_iarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_iarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_skeleton_oarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_skeleton_iarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
    template void serialize( boost::archive::binary_oarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_oarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
    template void serialize( boost::mpi::detail::content_oarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
    template void serialize( boost::mpi::detail::mpi_datatype_oarchive&, SusceptibilityEnvironmental &obj, unsigned int file_version );
}
#endif

#endif // ENABLE_POLIO
