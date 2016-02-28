/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <stdint.h>
#include <list>
#include <vector>

#include "IVectorHabitat.h"
#include "IArchive.h"

namespace Kernel
{
    struct INodeContext;
    class  SimulationConfig;

    class VectorHabitat : public IVectorHabitat
    {
    public:
        static IVectorHabitat* CreateHabitat( VectorHabitatType::Enum type, float max_capacity );
        virtual ~VectorHabitat();
        virtual void Update(float dt, INodeContext* node) override;

        virtual VectorHabitatType::Enum  GetVectorHabitatType()                   const override;
        virtual float                    GetMaximumLarvalCapacity()               const override;
        virtual float                    GetCurrentLarvalCapacity()               const override;
        virtual int32_t                  GetTotalLarvaCount(TimeStepIndex index)  const override;

        virtual void                     AddLarva(int32_t larva, float progress) override;
        virtual void                     AddEggs(int32_t eggs) override;
        virtual void                     IncrementMaxLarvalCapacity(float) override;

        virtual float                    GetOvipositionTrapKilling()     const override;
        virtual float                    GetArtificialLarvalMortality()  const override;
        virtual float                    GetLarvicideHabitatScaling()    const override;
        virtual float                    GetRainfallMortality()          const override;
        virtual float                    GetEggCrowdingCorrection()      const override;

        virtual float GetLocalLarvalGrowthModifier() const override;
        virtual float GetLocalLarvalMortality(float species_aquatic_mortality, float progress) const override;

    protected:
        explicit VectorHabitat();
        VectorHabitat( VectorHabitatType::Enum type, float max_capacity );

        virtual QueryResult QueryInterface( iid_t, void** ) override { return e_NOINTERFACE; }
        virtual int32_t AddRef() override { return 1; }
        virtual int32_t Release() override { return 0; }

        void CalculateEggCrowdingCorrection();
        void UpdateCurrentLarvalCapacity(float dt, INodeContext* node);
        void UpdateLarvalProbabilities(float dt, INodeContext* node);
        void UpdateRainfallMortality(float dt, float rainfall);
        void AdvanceTotalLarvaCounts();

        const SimulationConfig* params() const;

        DECLARE_SERIALIZABLE(VectorHabitat);

        VectorHabitatType::Enum  m_habitat_type;
        float                    m_max_larval_capacity;
        float                    m_current_larval_capacity;
        std::vector<int32_t>     m_total_larva_count;
        int32_t                  m_new_egg_count;

        float                    m_oviposition_trap_killing;
        float                    m_artificial_larval_mortality;
        float                    m_larvicide_habitat_scaling;
        float                    m_rainfall_mortality;
        float                    m_egg_crowding_correction;
    };

    void serialize(IArchive&, map<VectorHabitatType::Enum, float>&);
}
