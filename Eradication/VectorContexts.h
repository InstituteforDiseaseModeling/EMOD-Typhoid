/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Contexts.h"
#include "VectorEnums.h"

namespace Kernel
{
    struct VectorCohort;
    class  VectorHabitat;
    class  VectorMatingStructure;
    class  VectorPopulation;
    class  VectorProbabilities;

    typedef std::list<VectorPopulation *> VectorPopulationList_t;

    struct IVectorSimulationContext : public ISupports
    {
        virtual void  PostMigratingVector(VectorCohort* ind) = 0;
    };

    struct IVectorNodeContext : public ISupports
    {
        virtual VectorProbabilities* GetVectorLifecycleProbabilities() = 0;
        virtual VectorHabitat*       GetVectorHabitatByType(VectorHabitatType::Enum type) = 0;
        virtual void                 AddVectorHabitat(VectorHabitat* habitat) = 0;
        virtual float                GetLarvalHabitatMultiplier(VectorHabitatType::Enum type) const = 0;
    };

    // TODO: merge the two NodeVector interfaces?  or split functionally?
    class INodeVector : public ISupports
    {
    public:
        virtual const VectorPopulationList_t& GetVectorPopulations() = 0;
        virtual void AddVectors(std::string releasedSpecies, VectorMatingStructure _vector_genetics, unsigned long int releasedNumber) = 0;
        virtual void processImmigratingVector( VectorCohort* immigrant ) = 0;
    };

    struct IIndividualHumanVectorContext : public ISupports
    {
        virtual float GetRelativeBitingRate(void) const = 0;
    };

    struct IVectorSusceptibilityContext : public ISupports
    {
        virtual float GetRelativeBitingRate(void) const = 0;
    };

    struct IVectorInterventionsEffects : ISupports
    {
        virtual float GetDieBeforeFeeding() = 0;
        virtual float GetHostNotAvailable() = 0;
        virtual float GetDieDuringFeeding() = 0;
        virtual float GetDiePostFeeding() = 0;
        virtual float GetSuccessfulFeedHuman() = 0;
        virtual float GetSuccessfulFeedAD() = 0;
        virtual float GetOutdoorDieBeforeFeeding() = 0;
        virtual float GetOutdoorHostNotAvailable() = 0;
        virtual float GetOutdoorDieDuringFeeding() = 0;
        virtual float GetOutdoorDiePostFeeding() = 0;
        virtual float GetOutdoorSuccessfulFeedHuman() = 0;
        virtual float GetblockIndoorVectorAcquire() = 0;
        virtual float GetblockIndoorVectorTransmit() = 0;
        virtual float GetblockOutdoorVectorAcquire() = 0;
        virtual float GetblockOutdoorVectorTransmit() = 0;
        virtual ~IVectorInterventionsEffects() { }
    };

    struct INodeVectorInterventionEffects : ISupports
    {
        virtual float GetLarvalKilling(VectorHabitatType::Enum) = 0;
        virtual float GetLarvalHabitatReduction(VectorHabitatType::Enum) = 0;
        virtual float GetVillageSpatialRepellent() = 0;
        virtual float GetADIVAttraction() = 0;
        virtual float GetADOVAttraction() = 0;
        virtual float GetPFVKill() = 0;
        virtual float GetOutdoorKilling() = 0;
        virtual float GetOutdoorKillingMale() = 0;
        virtual float GetSugarFeedKilling() = 0;
        virtual float GetOviTrapKilling(VectorHabitatType::Enum) = 0;
        virtual float GetAnimalFeedKilling() = 0;
        virtual float GetOutdoorRestKilling() = 0;
    };
}
