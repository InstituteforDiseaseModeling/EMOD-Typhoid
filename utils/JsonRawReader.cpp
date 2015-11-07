#include "stdafx.h"
#include "JsonRawReader.h"

JsonRawReader::JsonRawReader(const char* data)
    : m_document(new rapidjson::Document())
    , m_json(m_document)
    , m_index(0)
    , m_index_stack()
    , m_value_stack()
{
    m_document->Parse<0>(data);
}

IArchive& JsonRawReader::startElement()
{
    assert((*m_json)[m_index].IsArray());
    m_value_stack.push(m_json);
    m_json = &(*m_json)[m_index];
    m_index_stack.push(m_index);
    m_index = 0;

    return *this;
}

IArchive& JsonRawReader::endElement()
{
    m_json = m_value_stack.top();
    m_value_stack.pop();
    m_index = m_index_stack.top();
    m_index_stack.pop();

    m_index++;

    return *this;
}

IArchive& JsonRawReader::labelElement(char*)
{
//    ++m_index;

    return *this;
}

IArchive& JsonRawReader::operator&(bool& b)
{
    b = (*m_json)[m_index++].GetBool();

    return *this;
}

IArchive& JsonRawReader::operator&(int32_t& i32)
{
    i32 = (*m_json)[m_index++].GetInt();

    return *this;
}

IArchive& JsonRawReader::operator&(int64_t& i64)
{
    i64 = (*m_json)[m_index++].GetInt64();
    return *this;
}

IArchive& JsonRawReader::operator&(uint32_t& u32)
{
    u32 = (*m_json)[m_index++].GetUint();

    return *this;
}

IArchive& JsonRawReader::operator&(uint64_t& u64)
{
    u64 = (*m_json)[m_index++].GetUint64();

    return *this;
}

IArchive& JsonRawReader::operator&(float& f)
{
    f = float((*m_json)[m_index++].GetDouble());

    return *this;
}

IArchive& JsonRawReader::operator&(double& d)
{
    d = (*m_json)[m_index++].GetDouble();

    return *this;
}

IArchive& JsonRawReader::operator&(std::string& s)
{
    s.assign((*m_json)[m_index].GetString(), (*m_json)[m_index].GetStringLength());
    ++m_index;

    return *this;
}

bool JsonRawReader::HasError() { return m_document->HasParseError(); }

bool JsonRawReader::IsWriter() { return false; }

size_t JsonRawReader::GetBufferSize()
{
    throw "JsonRawReader doesn't implement GetBufferSize().";
}

const char* JsonRawReader::GetBuffer()
{
    throw "JsonRawReader doesn't implement GetBuffer().";
}

JsonRawReader::~JsonRawReader()
{
    delete m_document;
    m_json = m_document = reinterpret_cast<rapidjson::Document*>(0xDEADBEEFLL);
}


void JsonRawReader::serialize(bool array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}

void JsonRawReader::serialize(int32_t array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}

void JsonRawReader::serialize(int64_t array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}

void JsonRawReader::serialize(float array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}
