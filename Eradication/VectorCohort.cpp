/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "VectorCohort.h"
#include "Exceptions.h"
#include "Node.h"
#include "StrainIdentity.h"

static const char * _module = "VectorCohort";

namespace Kernel
{
    // QI stuff
    BEGIN_QUERY_INTERFACE_BODY(VectorCohort)
        HANDLE_INTERFACE(IVectorCohort)
        HANDLE_INTERFACE(IMigrate)
        HANDLE_ISUPPORTS_VIA(IVectorCohort)
    END_QUERY_INTERFACE_BODY(VectorCohort)

    VectorCohort::VectorCohort() : vector_genetics( VectorMatingStructure() ), progress(0.0), population(DEFAULT_VECTOR_COHORT_SIZE)
    {
    }

    VectorCohort::VectorCohort(float _progress, uint32_t _population, VectorMatingStructure _vector_genetics)
        : vector_genetics(_vector_genetics)
        , progress(_progress)
        , population(_population)
    {
    }

    void VectorCohort::Initialize()
    {
    }

    VectorCohort *VectorCohort::CreateCohort(float progress, uint32_t population, VectorMatingStructure vector_genetics)
    {
        VectorCohort *newqueue = _new_ VectorCohort(progress, population, vector_genetics);
        newqueue->Initialize();

        return newqueue;
    }

    const StrainIdentity* VectorCohort::GetStrainIdentity() const
    {
        // dummy strain identity for cohort model
        // derived VectorCohortIndividual will actually keep track of strains
        // memory is freed after contagion is queued in VectorPopulation::ProcessFeedingCycle
        return _new_ StrainIdentity();
    }

    void VectorCohort::ImmigrateTo(Node* destination_node)
    {
        throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Vector migration only currently supported for individual (not cohort) model." );
    }

    void VectorCohort::SetMigrationDestination(suids::suid destination)
    {
        throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Vector migration only currently supported for individual (not cohort) model." );
    }

    const suids::suid& VectorCohort::GetMigrationDestination()
    {
        throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Vector migration only currently supported for individual (not cohort) model." );
    }

    VectorCohort::~VectorCohort()
    {
    }

    int32_t VectorCohort::GetPopulation() const
    {
        return population;
    }

    void VectorCohort::SetPopulation(
        int32_t new_pop
    )
    {
        population = new_pop;
    }

    double 
    VectorCohort::GetProgress() const
    {
        return progress;
    }

    void
    VectorCohort::ClearProgress()
    {
        progress = 0;
    }

    void
    VectorCohort::IncreaseProgress( double delta )
    {
        progress += delta;
    }

    VectorMatingStructure& 
    VectorCohort::GetVectorGenetics()
    {
        return vector_genetics;
    }

    void
    VectorCohort::SetVectorGenetics( const VectorMatingStructure& new_value )
    {
        vector_genetics = new_value;
    }

    /*VectorCohort::~VectorCohort()
    {
        LOG_VALID( "VectorCohort destructor.\n" );
    }*/

    float
    VectorCohort::GetMortality( uint32_t addition ) const
    {
        return addition;
        //throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "VectorCohort::GetMortality" );
    }

    IMPLEMENT_SERIALIZATION_REGISTRAR(IVectorCohort);
    REGISTER_SERIALIZABLE(VectorCohort, IVectorCohort);

    void VectorCohort::serialize(IArchive& ar, IVectorCohort* obj)
    {
        VectorCohort& cohort = *dynamic_cast<VectorCohort*>(obj);
        ar.startElement();
        ar.labelElement("vector_genetics");
        VectorMatingStructure::serialize(ar, cohort.vector_genetics);
        ar.labelElement("progress") & cohort.progress;
        ar.labelElement("population") & cohort.population;
        ar.endElement();
    }

    void IVectorCohort::serialize(IArchive& ar, std::vector<IVectorCohort*>& cohorts)
    {
        size_t count = ar.IsWriter() ? cohorts.size() : -1;

        ar.startElement();
            ar.labelElement("__count__") & count;
            if (count > 0)
            {
                ar.labelElement("__vector__");

                if (ar.IsWriter())
                {
                    for (auto cohort : cohorts)
                    {
                        Kernel::serialize<IVectorCohort>(ar, cohort);
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        IVectorCohort* cohort;
                        Kernel::serialize<IVectorCohort>(ar, cohort);
                        cohorts.push_back(cohort);
                    }
                }
            }
        ar.endElement();
    }

    void VectorCohort::serialize(IArchive& ar, VectorCohortList_t& list)
    {
        size_t count = ar.IsWriter() ? list.size() : -1;

        ar.startElement();
            ar.labelElement("__count__") & count;
            if (count > 0)
            {
                ar.labelElement("__vector__");

                if (ar.IsWriter())
                {
                    for (auto cohort : list)
                    {
                        serialize(ar, cohort);
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        VectorCohort* cohort = new VectorCohort();
                        serialize(ar, cohort);
                        list.push_back(cohort);
                    }
                }
            }
        ar.endElement();
    }

#if 0
    template<class Archive>
    void VectorCohort::serialize_inner( Archive & ar, const unsigned int file_version )
    {
        // Register derived types - N/A

        // Serialize fields
        typemap.serialize(this, ar, file_version);

        // Serialize base class - N/A
    }

    template void VectorCohort::serialize_inner( boost::archive::binary_iarchive & ar, const unsigned int file_version );
    template void VectorCohort::serialize_inner( boost::archive::binary_oarchive & ar, const unsigned int file_version );
#endif
}


#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::VectorCohort)
namespace Kernel {
    template< typename Archive >
    void serialize( Archive& ar, VectorCohort &obj, unsigned int file_version )
    {
        ar & obj.progress;
        ar & obj.population;
        ar & obj.vector_genetics;
    }
    template void serialize( boost::mpi::packed_skeleton_iarchive&, VectorCohort &obj, unsigned int file_version );
    template void serialize( boost::archive::binary_iarchive&, VectorCohort &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_iarchive&, VectorCohort &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_skeleton_oarchive&, VectorCohort &obj, unsigned int file_version );
    template void serialize( boost::archive::binary_oarchive&, VectorCohort &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_oarchive&, VectorCohort &obj, unsigned int file_version );
    template void serialize( boost::mpi::detail::content_oarchive&, VectorCohort &obj, unsigned int file_version );
    template void serialize( boost::mpi::detail::mpi_datatype_oarchive&, VectorCohort &obj, unsigned int file_version );
}
#endif
