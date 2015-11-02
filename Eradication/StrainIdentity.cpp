/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "StrainIdentity.h"
#include "Log.h"

Kernel::StrainIdentity::StrainIdentity(void)
{
    antigenID = 0;
    geneticID = 0;
}

Kernel::StrainIdentity::StrainIdentity(int initial_antigen, int initial_genome)
{
    antigenID = initial_antigen;
    geneticID = initial_genome;
}

Kernel::StrainIdentity::~StrainIdentity(void)
{
}

int Kernel::StrainIdentity::GetAntigenID(void) const
{
    return antigenID;
}

int Kernel::StrainIdentity::GetGeneticID(void) const
{
    return geneticID;
}

void Kernel::StrainIdentity::SetAntigenID(int in_antigenID)
{
    antigenID = in_antigenID;
}

void Kernel::StrainIdentity::SetGeneticID(int in_geneticID)
{
    geneticID = in_geneticID;
}

namespace Kernel
{
    IArchive& serialize(IArchive& ar, StrainIdentity*& ptr)
    {
        if (!ar.IsWriter())
        {
            ptr = new StrainIdentity();
        }

        StrainIdentity& strain = *ptr;

        ar.startElement();
            ar.labelElement("antigenID") & strain.antigenID;
            ar.labelElement("geneticID") & strain.geneticID;
        ar.endElement();

        return ar;
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::StrainIdentity)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, StrainIdentity& strain, unsigned int  file_version )
    {
        static const char * _module = "StrainIdentity";
        LOG_DEBUG("(De)serializing StrainIdentity\n");

        ar & strain.antigenID;
        ar & strain.geneticID;
    }
    template void serialize( boost::mpi::packed_oarchive&, Kernel::StrainIdentity&, unsigned int);
    template void serialize( boost::archive::binary_oarchive&, Kernel::StrainIdentity&, unsigned int);
    template void serialize( boost::mpi::detail::content_oarchive&, Kernel::StrainIdentity&, unsigned int);
    template void serialize( boost::mpi::detail::mpi_datatype_oarchive&, Kernel::StrainIdentity&, unsigned int);
    template void serialize( boost::mpi::packed_skeleton_oarchive&, Kernel::StrainIdentity&, unsigned int);
    template void serialize( boost::mpi::packed_iarchive&, Kernel::StrainIdentity&, unsigned int);
    template void serialize( boost::archive::binary_iarchive&, Kernel::StrainIdentity&, unsigned int);
    template void serialize( boost::mpi::packed_skeleton_iarchive&, Kernel::StrainIdentity&, unsigned int);

}
#endif
