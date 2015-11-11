#include "stdafx.h"
#include "IArchive.h"

IArchive& IArchive::operator & (std::vector<int32_t>& vec)
{
    size_t count = this->IsWriter() ? vec.size() : -1;

    this->startElement();
        this->labelElement("__count__") & count;
        if (count > 0)
        {
            this->labelElement("__vector__");
            this->startElement();
            if (this->IsWriter())
            {
                for (auto& entry : vec)
                {
                    *this & entry;
                }
            }
            else
            {
                vec.clear();
                for (size_t i = 0; i < count; ++i)
                {
                    int32_t value;
                    *this & value;
                    vec.push_back(value);
                }
            }
            this->endElement();
        }
    this->endElement();

    return *this;
}

IArchive& IArchive::operator&(std::vector<uint32_t>& vec)
{
    size_t count = this->IsWriter() ? vec.size() : -1;

    this->startElement();
        this->labelElement("__count__") & count;
        if (count > 0)
        {
            this->labelElement("__vector__");
            this->startElement();
            if (this->IsWriter())
            {
                for (auto& entry : vec)
                {
                    *this & entry;
                }
            }
            else
            {
                vec.clear();
                for (size_t i = 0; i < count; ++i)
                {
                    uint32_t value;
                    *this & value;
                    vec.push_back(value);
                }
            }
            this->endElement();
        }
    this->endElement();

    return *this;
}

IArchive& IArchive::operator&(std::vector<int64_t>& vec)
{
    size_t count = this->IsWriter() ? vec.size() : -1;

    this->startElement();
        this->labelElement("__count__") & count;
        if (count > 0)
        {
            this->labelElement("__vector__");
            this->startElement();
            if (this->IsWriter())
            {
                for (auto& entry : vec)
                {
                    *this & entry;
                }
            }
            else
            {
                vec.clear();
                for (size_t i = 0; i < count; ++i)
                {
                    int64_t value;
                    *this & value;
                    vec.push_back(value);
                }
            }
            this->endElement();
        }
    this->endElement();

    return *this;
}

IArchive& IArchive::operator & (std::vector<float>& vec)
{
    size_t count = this->IsWriter() ? vec.size() : -1;

    this->startElement();
        this->labelElement("__count__") & count;
        if (count > 0)
        {
            this->labelElement("__vector__");
            this->startElement();
            if (this->IsWriter())
            {
                for (auto& entry : vec)
                {
                    *this & entry;
                }
            }
            else
            {
                vec.clear();
                for (size_t i = 0; i < count; ++i)
                {
                    float value;
                    *this & value;
                    vec.push_back(value);
                }
            }
            this->endElement();
        }
    this->endElement();

    return *this;
}

IArchive& IArchive::operator & (std::map<std::string, float>& mapping)
{
    size_t count = this->IsWriter() ? mapping.size() : -1;

    this->startElement();
        this->labelElement("__count__") & count;
        if (count> 0)
        {
            this->labelElement("__map__");
            this->startElement();
            if (this->IsWriter())
            {
                for (auto& entry : mapping)
                {
                    std::string key = entry.first;
                    float value = entry.second;
                    *this & key;
                    *this & value;
                }
            }
            else
            {
                for (size_t i = 0; i < count; ++i)
                {
                    std::string key;
                    float value;
                    *this & key;
                    *this & value;
                    mapping[key] = value;
                }
            }
            this->endElement();
        }

    this->endElement();

    return *this;
}

IArchive& IArchive::operator & (std::vector<Kernel::suids::suid>& vec)
{
    size_t count = this->IsWriter() ? vec.size() : -1;

    this->startElement();
        this->labelElement("__count__") & count;
        if (count > 0)
        {
            this->labelElement("__vector__");
            this->startElement();
            if (this->IsWriter())
            {
                for (auto& entry : vec)
                {
                    *this & entry.data;
                }
            }
            else
            {
                vec.clear();
                for (size_t i = 0; i < count; ++i)
                {
                    Kernel::suids::suid value;
                    *this & value.data;
                    vec.push_back(value);
                }
            }
            this->endElement();
        }
    this->endElement();

    return *this;
}

IArchive& IArchive::operator & (std::map<std::string, std::string>& mapping)
{
    size_t count = this->IsWriter() ? mapping.size() : -1;

    this->startElement();
        this->labelElement("__count__") & count;
        if (count> 0)
        {
            this->labelElement("__map__");
            this->startElement();
            if (this->IsWriter())
            {
                for (auto& entry : mapping)
                {
                    std::string key = entry.first;
                    std::string value = entry.second;
                    *this & key;
                    *this & value;
                }
            }
            else
            {
                for (size_t i = 0; i < count; ++i)
                {
                    std::string key;
                    std::string value;
                    *this & key;
                    *this & value;
                    mapping[key] = value;
                }
            }
            this->endElement();
        }

    this->endElement();

    return *this;
}
