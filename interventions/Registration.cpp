#include "Bednet.h"
#include "HousingModification.h"
#include "Vaccine.h"

/* Factory registration */

IMPLEMENT_FACTORY_REGISTERED(Kernel::SimpleBednet)
// clorton IMPLEMENT_FACTORY_REGISTERED(Kernel::IRSHousingModification)
// clorton IMPLEMENT_FACTORY_REGISTERED(Kernel::SimpleVaccine)

/* Boost serialization registration */
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI

BOOST_CLASS_EXPORT(Kernel::SimpleBednet)
// clorton BOOST_CLASS_EXPORT(Kernel::IRSHousingModification)
// clorton BOOST_CLASS_EXPORT(Kernel::SimpleVaccine)

#endif

/* JSON serialization registration */
#if USE_JSON_SERIALIZATION || USE_JSON_MPI

#endif
