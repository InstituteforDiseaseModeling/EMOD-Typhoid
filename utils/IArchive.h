#pragma once
#include <string>
#include <vector>
#include <map>
#include "suids.hpp"

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

    virtual IArchive& operator & (std::vector<int32_t>&);
    virtual IArchive& operator & (std::vector<uint32_t>&);
    virtual IArchive& operator & (std::vector<int64_t>&);
    virtual IArchive& operator & (std::vector<float>&);

    virtual IArchive& operator & (std::map<std::string, float>&);

    // IDM specific types
    virtual IArchive& operator & (std::vector<Kernel::suids::suid>&);
    virtual IArchive& operator & (std::map<std::string, std::string>&);

    virtual void serialize(bool array[], size_t count) = 0;
    virtual void serialize(int32_t array[], size_t count) = 0;
    virtual void serialize(int64_t array[], size_t count) = 0;
    virtual void serialize(float array[], size_t count) = 0;

    virtual bool IsWriter() = 0;
    virtual size_t GetBufferSize() = 0;
    virtual const char* GetBuffer() = 0;
};
