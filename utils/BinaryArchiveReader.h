#pragma once
#include "IArchive.h"

namespace Kernel
{
    class BinaryArchiveReader : public IArchive
    {
    public:
        explicit BinaryArchiveReader(const char*, size_t);
        virtual ~BinaryArchiveReader();

    private:
        virtual IArchive& startClass(std::string&) override;
        virtual IArchive& endClass() override;
        virtual IArchive& startObject() override;
        virtual IArchive& endObject() override;
        virtual IArchive& startArray(size_t&) override;
        virtual IArchive& endArray() override;
        virtual IArchive& labelElement(const char*) override;
        virtual IArchive& operator&(bool&) override;
        virtual IArchive& operator&(int32_t&) override;
        virtual IArchive& operator&(int64_t&) override;
        virtual IArchive& operator&(uint32_t&) override;
        virtual IArchive& operator&(uint64_t&) override;
        virtual IArchive& operator&(float&) override;
        virtual IArchive& operator&(double&) override;
        virtual IArchive& operator&(std::string&) override;

        virtual bool HasError() override;
        virtual bool IsWriter() override;
        virtual size_t GetBufferSize() override;
        virtual const char* GetBuffer() override;

        const char* buffer;
        size_t capacity;
        size_t count;
        bool error;

        template<typename T>
        void pop(T& element)
        {
            if ( (count + sizeof(T)) > capacity )
            {
                error = true;
                return;
            }

            element = *reinterpret_cast<const T*>(buffer + count);
            count += sizeof(T);

            return;
        }
    };
}