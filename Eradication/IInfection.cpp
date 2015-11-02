#include "stdafx.h"
#include "IInfection.h"

namespace Kernel
{
    IMPLEMENT_SERIALIZATION_REGISTRAR(IInfection);

    IArchive& serialize(IArchive& ar, infection_list_t& infections)
    {
        size_t count = ar.IsWriter() ? infections.size() : -1;

        ar.startElement();
            ar.labelElement("__count__") & count;
            if (count > 0)
            {
                ar.labelElement("__list__");
                if (ar.IsWriter())
                {
                    for (auto infection : infections)
                    {
                        Kernel::serialize<IInfection>(ar, infection);
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        IInfection* infection;
                        Kernel::serialize<IInfection>(ar, infection);
                        infections.push_back(infection);
                    }
                }
            }
        ar.endElement();

        return ar;
    }
}
