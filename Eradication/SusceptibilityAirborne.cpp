/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TB

#include "SusceptibilityAirborne.h"

namespace Kernel
{
    SusceptibilityAirborne *SusceptibilityAirborne::CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod)
    {
        SusceptibilityAirborne *newsusceptibility = _new_ SusceptibilityAirborne(context);
        newsusceptibility->Initialize(age, immmod, riskmod);

        return newsusceptibility;
    }

    SusceptibilityAirborne::~SusceptibilityAirborne(void) { }
    SusceptibilityAirborne::SusceptibilityAirborne() { }
    SusceptibilityAirborne::SusceptibilityAirborne(IIndividualHumanContext *context) : Susceptibility(context) { }

    void SusceptibilityAirborne::Initialize(float _age, float _immmod, float _riskmod)
    {
        Susceptibility::Initialize(_age, _immmod, _riskmod);

        // TODO: what are we doing here? 
        // initialize members of airborne susceptibility below
        demographic_risk = _riskmod; 
    }

    REGISTER_SERIALIZABLE(SusceptibilityAirborne, ISusceptibilityContext);

    void SusceptibilityAirborne::serialize(IArchive& ar, ISusceptibilityContext* obj)
    {
        Susceptibility::serialize(ar, obj);
        SusceptibilityAirborne& susceptibility = *dynamic_cast<SusceptibilityAirborne*>(obj);
        ar.startElement();
        ar.labelElement("demographic_risk") & susceptibility.demographic_risk;
        ar.endElement();
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::SusceptibilityAirborne)
namespace Kernel {
    template<class Archive>
    void serialize(Archive & ar, SusceptibilityAirborne& sus, const unsigned int file_version )
    {
        ar & sus.demographic_risk;
        ar & boost::serialization::base_object<Susceptibility>(sus);
    }
    template void serialize( boost::mpi::packed_skeleton_oarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
    template void serialize( boost::mpi::detail::content_oarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
    template void serialize( boost::mpi::packed_oarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
    template void serialize( boost::mpi::detail::mpi_datatype_oarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
    template void serialize( boost::archive::binary_oarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
    template void serialize( boost::archive::binary_iarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
    template void serialize( boost::mpi::packed_iarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
    template void serialize( boost::mpi::packed_skeleton_iarchive&, Kernel::SusceptibilityAirborne&, unsigned int);
}
#endif

#endif // ENABLE_TB
