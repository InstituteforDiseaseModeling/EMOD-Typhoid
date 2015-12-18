#pragma once
#include "IArchive.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace Kernel
{
    class IDMAPI JsonFullWriter : public IArchive
    {
    public:
        explicit JsonFullWriter();
        virtual ~JsonFullWriter();

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
        virtual IArchive& operator & (jsonConfigurable::ConstrainedString&) override;

        virtual bool HasError() override;
        virtual bool IsWriter() override;
        virtual uint32_t GetBufferSize() override;
        virtual const char* GetBuffer() override;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        rapidjson::StringBuffer* m_buffer;
        rapidjson::Writer<rapidjson::StringBuffer>* m_writer;
        bool m_closed;
#pragma warning( pop )
    };
}
