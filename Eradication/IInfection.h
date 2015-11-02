#pragma once
#include "ISusceptibilityContext.h"
#include "Common.h"             // InfectionStateChange
#include "StrainIdentity.h"     // TODO- Use IStrainIdentity (doesn't exist yet)
#include "Types.h"              // NonNegativeFloat

namespace Kernel
{
    struct IInfection;

    typedef std::list<IInfection*> infection_list_t;

    struct IInfection : ISerializable
    {
        virtual void Update(float, ISusceptibilityContext* = nullptr) = 0;
        virtual InfectionStateChange::_enum GetStateChange() const = 0;
        virtual float GetInfectiousness() const = 0;
        virtual float GetInfectiousnessByRoute(std::string route) const = 0;
        virtual float GetInfectiousPeriod() const = 0;
        virtual void GetInfectiousStrainID(StrainIdentity*) = 0;
        virtual bool IsActive() const = 0;
        virtual NonNegativeFloat GetDuration() const = 0;

        virtual ~IInfection() {}

        friend IArchive& serialize(IArchive&, infection_list_t&);
        DECLARE_SERIALIZATION_REGISTRAR(IInfection);
    };
}