#pragma once
#include "ISerializable.h"

namespace Kernel
{
    struct ISusceptibilityContext : ISerializable
    {
        virtual float getAge() const = 0;
        virtual float getModAcquire() const = 0;
        virtual float GetModTransmit() const = 0;
        virtual float getModMortality() const = 0;

        virtual ~ISusceptibilityContext() {}

        DECLARE_SERIALIZATION_REGISTRAR(ISusceptibilityContext);
    };
}