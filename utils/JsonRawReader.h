#pragma once
#include "IArchive.h"
#include "rapidjson/document.h"
#include <stack>

class JsonRawReader : public IArchive
{
public:
    explicit JsonRawReader(const char*);
    virtual ~JsonRawReader();

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

    virtual void serialize(bool array[], size_t count);
    virtual void serialize(int32_t array[], size_t count);
    virtual void serialize(int64_t array[], size_t count);
    virtual void serialize(float array[], size_t count);

    virtual bool HasError() override;
    virtual bool IsWriter() override;
    virtual size_t GetBufferSize() override;
    virtual const char* GetBuffer() override;

    rapidjson::Document* m_document;
    rapidjson::GenericValue<rapidjson::UTF8<>>* m_json;
    uint32_t m_index;
    std::stack<uint32_t> m_index_stack;
    std::stack<rapidjson::GenericValue<rapidjson::UTF8<>>*> m_value_stack;
};