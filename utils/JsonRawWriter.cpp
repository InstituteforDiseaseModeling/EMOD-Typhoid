#include "stdafx.h"
#include "JsonRawWriter.h"

JsonRawWriter::JsonRawWriter()
    : m_buffer(new rapidjson::StringBuffer())
    , m_writer(new rapidjson::Writer<rapidjson::StringBuffer>(*m_buffer))
    , m_closed(false)
{
    m_writer->StartArray();
}

IArchive& JsonRawWriter::startElement()
{
    m_writer->StartArray();

    return *this;
}

IArchive& JsonRawWriter::endElement()
{
    m_writer->EndArray();

    return *this;
}

IArchive& JsonRawWriter::labelElement(char*)
{
    return *this;
}

IArchive& JsonRawWriter::operator&(bool& b)
{
    m_writer->Bool(b);

    return *this;
}

IArchive& JsonRawWriter::operator&(int32_t& i32)
{
    m_writer->Int(i32);

    return *this;
}

IArchive& JsonRawWriter::operator&(int64_t& i64)
{
    m_writer->Int64(i64);
    return *this;
}

IArchive& JsonRawWriter::operator&(uint32_t& u32)
{
    m_writer->Uint64(u32);

    return *this;
}

IArchive& JsonRawWriter::operator&(uint64_t& u64)
{
    m_writer->Uint64(u64);

    return *this;
}

IArchive& JsonRawWriter::operator&(float& f)
{
    m_writer->Double(double(f));

    return *this;
}

IArchive& JsonRawWriter::operator&(double& d)
{
    m_writer->Double(d);

    return *this;
}

IArchive& JsonRawWriter::operator&(std::string& s)
{
    m_writer->String(s.c_str(), rapidjson::SizeType(s.size()), true);

    return *this;
}

bool JsonRawWriter::IsWriter() { return true; }

size_t JsonRawWriter::GetBufferSize()
{
    return strlen(GetBuffer()) + 1;
}

const char* JsonRawWriter::GetBuffer()
{
    if (!m_closed)
    {
        m_writer->EndArray();
        m_closed = true;
    }

    return m_buffer->GetString();
}

JsonRawWriter::~JsonRawWriter()
{
    delete m_buffer;
    delete m_writer;
}


void JsonRawWriter::serialize(bool array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}

void JsonRawWriter::serialize(int32_t array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}

void JsonRawWriter::serialize(int64_t array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}

void JsonRawWriter::serialize(float array[], size_t count)
{
    startElement();
    for (size_t i = 0; i < count; ++i)
    {
        (*this) & array[i];
    }
    endElement();
}
