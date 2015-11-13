#pragma once
#include <string>
#include <vector>
#include <map>
#include "suids.hpp"

#include "ISerializable.h"

namespace Kernel
{
    struct IArchive
    {
        virtual ~IArchive() {}

        virtual IArchive& startElement() = 0;
        virtual IArchive& endElement() = 0;

        virtual IArchive& labelElement(char*) = 0;

        virtual IArchive& operator & (bool&) = 0;
        virtual IArchive& operator & (int32_t&) = 0;
        virtual IArchive& operator & (int64_t&) = 0;
        virtual IArchive& operator & (uint32_t&) = 0;
        virtual IArchive& operator & (uint64_t&) = 0;
        virtual IArchive& operator & (float&) = 0;
        virtual IArchive& operator & (double&) = 0;
        virtual IArchive& operator & (std::string&) = 0;

        IArchive& operator & (ISerializable*&);

        template<typename I>
        void operator & (I*& ptr)
        {
            if (this->IsWriter())
            {
                ISerializable* serializable = dynamic_cast<ISerializable*>(ptr);
                (*this) & serializable;
            }
            else
            {
                ISerializable* serializable;
                (*this) & serializable;
                ptr = dynamic_cast<I*>(serializable);
            }
        }

        template <typename I>
        void operator & (std::list<I*>& list)
        {
            size_t count = this->IsWriter() ? list.size() : -1;

            this->startElement();
            this->labelElement("__count__") & count;
            if (count > 0)
            {
                this->labelElement("__list__");
                this->startElement();
                if (this->IsWriter())
                {
                    for (auto& entry : list)
                    {
                        ISerializable* serializable = dynamic_cast<ISerializable*>(entry);
                        (*this) & serializable;
                    }
                }
                else
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        ISerializable* serializable;
                        (*this) & serializable;
                        list.push_back(dynamic_cast<I*>(serializable));
                    }
                }
                this->endElement();
            }
            this->endElement();
        }

        template <typename T>
        void operator & (std::vector<T>& vec)
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
                            (*this) & entry;
                        }
                    }
                    else
                    {
                        vec.clear();
                        for (size_t i = 0; i < count; ++i)
                        {
                            T value;
                            (*this) & value;
                            vec.push_back(value);
                        }
                    }
                    this->endElement();
                }
            this->endElement();
        }

    //    virtual IArchive& operator & (std::vector<int32_t>&);
    //    virtual IArchive& operator & (std::vector<uint32_t>&);
    //    virtual IArchive& operator & (std::vector<int64_t>&);
    //    virtual IArchive& operator & (std::vector<float>&);

        /* virtual */ IArchive& operator & (std::map<std::string, float>&);

        // IDM specific types
        /* virtual */ IArchive& operator & (std::vector<Kernel::suids::suid>&);
        /* virtual */ IArchive& operator & (std::map<std::string, std::string>&);

    //    /* virtual */ void serialize(bool array[], size_t count) = 0;
    //    /* virtual */ void serialize(int32_t array[], size_t count) = 0;
    //    /* virtual */ void serialize(int64_t array[], size_t count) = 0;
    //    /* virtual */ void serialize(float array[], size_t count) = 0;

        template <typename A>
        void serialize(A array[], size_t count)
        {
            startElement();
            for (size_t i = 0; i < count; ++i)
            {
                (*this) & array[i];
            }
            endElement();
        }

        virtual bool HasError() = 0;
        virtual bool IsWriter() = 0;
        virtual size_t GetBufferSize() = 0;
        virtual const char* GetBuffer() = 0;
    };
}