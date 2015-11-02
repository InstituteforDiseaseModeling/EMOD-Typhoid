#include "stdafx.h"
#include "IIndividualHuman.h"

namespace Kernel
{
    IArchive& serialize(IArchive& ar, std::vector<IIndividualHuman*>& individuals)
    {
        size_t count = ar.IsWriter() ? individuals.size() : -1;

        ar.startElement();
            ar.labelElement("__count__") & count;
            if (count > 0)
            {
                ar.labelElement("__vector__");

                if (ar.IsWriter())
                {
                    for (auto individual : individuals)
                    {
                        Kernel::serialize<IIndividualHuman>(ar, individual);
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        IIndividualHuman* individual;
                        Kernel::serialize<IIndividualHuman>(ar, individual);
                        individuals.push_back(individual);
                    }
                }
            }
        ar.endElement();

        return ar;
    }

    IMPLEMENT_SERIALIZATION_REGISTRAR(IIndividualHuman);
}
