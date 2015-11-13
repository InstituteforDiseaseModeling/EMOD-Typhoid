#pragma once
#include "IArchive.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace Kernel
{
    class JsonRawWriter : public IArchive
    {
    public:
        explicit JsonRawWriter();
        virtual ~JsonRawWriter();

    private:

        virtual IArchive& startElement() override;
        virtual IArchive& endElement() override;
        virtual IArchive& labelElement(char*) override;

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

        rapidjson::StringBuffer* m_buffer;
        rapidjson::Writer<rapidjson::StringBuffer>* m_writer;
        bool m_closed;
    };
}
