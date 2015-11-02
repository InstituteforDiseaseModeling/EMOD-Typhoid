#pragma once

#ifndef DECLARE_TEMPLATES
#define DECLARE_TEMPLATES(_T) \
    template void serialize(boost::archive::binary_iarchive&, _T&, unsigned int); \
    template void serialize(boost::archive::binary_oarchive&, _T&, unsigned int); \
    template void serialize(boost::mpi::detail::content_oarchive&, _T&, unsigned int); \
    template void serialize(boost::mpi::detail::forward_skeleton_iarchive<boost::mpi::packed_skeleton_iarchive, boost::mpi::packed_iarchive>&, _T&, unsigned int); \
    template void serialize(boost::mpi::detail::forward_skeleton_oarchive<boost::mpi::packed_skeleton_oarchive, boost::mpi::packed_oarchive>&, _T&, unsigned int); \
    template void serialize(boost::mpi::detail::ignore_skeleton_oarchive<boost::mpi::detail::content_oarchive>&, _T&, unsigned int); \
    template void serialize(boost::mpi::detail::ignore_skeleton_oarchive<boost::mpi::detail::mpi_datatype_oarchive>&, _T&, unsigned int); \
    template void serialize(boost::mpi::detail::mpi_datatype_oarchive&, _T&, unsigned int); \
    template void serialize(boost::mpi::packed_iarchive&, _T&, unsigned int); \
    template void serialize(boost::mpi::packed_oarchive&, _T&, unsigned int); \
    template void serialize(boost::mpi::packed_skeleton_iarchive&, _T&, unsigned int); \
    template void serialize(boost::mpi::packed_skeleton_oarchive&, _T&, unsigned int); \

#endif
