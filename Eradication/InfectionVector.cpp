/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "InfectionVector.h"

static const char* _module = "InfectionVector";

namespace Kernel
{
    InfectionVector::InfectionVector() : Kernel::Infection()
    {
    }

    InfectionVector::InfectionVector(IIndividualHumanContext *context) : Kernel::Infection(context)
    {
    }

    void InfectionVector::Initialize(suids::suid _suid)
    {
        Kernel::Infection::Initialize(_suid);
    }

    InfectionVector *InfectionVector::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        InfectionVector *newinfection = _new_ InfectionVector(context);
        newinfection->Initialize(_suid);

        return newinfection;
    }

    InfectionVector::~InfectionVector()
    {
    }

    REGISTER_SERIALIZABLE(InfectionVector, IInfection);
// clorton     IMPLEMENT_POOL(InfectionVector);

    void InfectionVector::serialize(IArchive& ar, IInfection *obj)
    {
        Infection::serialize(ar, obj);
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::InfectionVector)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, InfectionVector& inf, unsigned int file_version )
    {
        ar & boost::serialization::base_object<Kernel::Infection>(inf);
    }
    template void serialize( boost::archive::binary_iarchive&, InfectionVector &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_iarchive&, InfectionVector &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_skeleton_oarchive&, InfectionVector &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_skeleton_iarchive&, InfectionVector &obj, unsigned int file_version );
    template void serialize( boost::archive::binary_oarchive&, InfectionVector &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_oarchive&, InfectionVector &obj, unsigned int file_version );
    template void serialize( boost::mpi::detail::content_oarchive&, InfectionVector &obj, unsigned int file_version );
    template void serialize( boost::mpi::detail::mpi_datatype_oarchive&, InfectionVector &obj, unsigned int file_version );
}
#endif
