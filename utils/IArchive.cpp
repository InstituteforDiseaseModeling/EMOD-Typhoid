#include "stdafx.h"
#include "IArchive.h"

namespace Kernel
{
    IArchive& IArchive::operator&(ISerializable*& obj)
    {
        ISerializable::serialize(*this, obj);

        return *this;
    }

    IArchive& IArchive::operator & (std::map<std::string, float>& mapping)
    {
        size_t count = this->IsWriter() ? mapping.size() : -1;

        this->startArray(count);
        if (this->IsWriter())
        {
            for (auto& entry : mapping)
            {
                std::string key = entry.first;
                float value = entry.second;
                startObject();
                    labelElement("key") & key;
                    labelElement("value") & value;
                endObject();
            }
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                std::string key;
                float value;
                startObject();
                    labelElement("key") & key;
                    labelElement("value") & value;
                endObject();
                mapping[key] = value;
            }
        }
        this->endArray();

        return *this;
    }

    IArchive& IArchive::operator & (std::map<float, float>& mapping)
    {
        size_t count = this->IsWriter() ? mapping.size() : -1;

        this->startArray(count);
        if (this->IsWriter())
        {
            for (auto& entry : mapping)
            {
                float key = entry.first;
                float value = entry.second;
                startObject();
                    labelElement("key") & key;
                    labelElement("value") & value;
                endObject();
            }
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                float key;
                float value;
                startObject();
                    labelElement("key") & key;
                    labelElement("value") & value;
                endObject();
                mapping[key] = value;
            }
        }
        this->endArray();

        return *this;
    }

    IArchive& IArchive::operator & (std::vector<Kernel::suids::suid>& vec)
    {
        size_t count = this->IsWriter() ? vec.size() : -1;

        this->startArray(count);
        if (this->IsWriter())
        {
            for (auto& entry : vec)
            {
                *this & entry.data;
            }
        }
        else
        {
            vec.resize(count);
            for (size_t i = 0; i < count; ++i)
            {
                Kernel::suids::suid value;
                *this & vec[i].data;
            }
        }
        this->endArray();

        return *this;
    }

    IArchive& IArchive::operator & (std::map<std::string, std::string>& mapping)
    {
        size_t count = this->IsWriter() ? mapping.size() : -1;

        this->startArray(count);
        if (this->IsWriter())
        {
            for (auto& entry : mapping)
            {
                std::string key = entry.first;
                std::string value = entry.second;
                startObject();
                    labelElement("key") & key;
                    labelElement("value") & value;
                endObject();
            }
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                std::string key;
                std::string value;
                startObject();
                    labelElement("key") & key;
                    labelElement("value") & value;
                endObject();
                mapping[key] = value;
            }
        }
        this->endArray();

        return *this;
    }
}
