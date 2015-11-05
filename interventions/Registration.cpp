#include "Bednet.h"
#include "HousingModification.h"
#include "Vaccine.h"

/* Factory registration */

IMPLEMENT_FACTORY_REGISTERED(Kernel::SimpleBednet)

/* Boost serialization registration */
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI

BOOST_CLASS_EXPORT(Kernel::SimpleBednet)

#endif

/* JSON serialization registration */
#if USE_JSON_SERIALIZATION || USE_JSON_MPI

#endif
