#pragma once
#include "IArchive.h"
#include "rapidjson/document.h"
#include <stack>

namespace Kernel
{
    class IDMAPI JsonFullReader : public IArchive
    {
    public:
        explicit JsonFullReader(const char*);
        virtual ~JsonFullReader();

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
        virtual uint32_t GetBufferSize() override;
        virtual const char* GetBuffer() override;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        rapidjson::Document* m_document;
        rapidjson::GenericValue<rapidjson::UTF8<>>* m_json;
        uint32_t m_index;
        bool isObject;
        std::string label;
        std::stack<uint32_t> m_index_stack;
        std::stack<rapidjson::GenericValue<rapidjson::UTF8<>>*> m_value_stack;
#pragma warning( pop )
    };
}