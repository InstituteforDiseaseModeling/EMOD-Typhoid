#include "stdafx.h"

#include "ISerializable.h"
#include "IArchive.h"

namespace Kernel
{
    SerializationRegistrar* SerializationRegistrar::_singleton = nullptr;

    void ISerializable::serialize(IArchive& ar, ISerializable*& obj)
    {
        std::string class_name = ar.IsWriter() ? obj->GetClassName() : "__UNK__";
        ar.startClass(class_name);
        auto serialize_function = SerializationRegistrar::_get_serializer(class_name);
        if (!ar.IsWriter())
        {
            auto constructor_function = SerializationRegistrar::_get_constructor(class_name);
            if( constructor_function == nullptr )
            {
                std::stringstream msg;
                msg << "Could not find constructor for class_name='" << class_name << "'";
                throw SerializationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
            obj = constructor_function();
        }
        serialize_function(ar, obj);
        ar.endObject();
    }
}