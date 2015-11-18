/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "Node.h"
#include "IndividualEnvironmental.h"
#include <math.h>

namespace Kernel
{
    class NodeEnvironmental : public Node
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        static NodeEnvironmental *CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid);
        NodeEnvironmental(ISimulationContext *_parent_sim, suids::suid node_suid);
        virtual ~NodeEnvironmental(void);

    protected:
        double contagion;
         // Environmental and Polio sims, unlike Generic, may have partial persistence of environmental contagion
        float  node_contagion_decay_fraction;

        NodeEnvironmental();
        virtual bool Configure( const Configuration* config );

        // Factory methods
        virtual IndividualHuman *createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty);

        // Effect of climate on infectivity in Environmental disease
        virtual float getClimateInfectivityCorrection()  const;


        virtual void SetupIntranodeTransmission();
        virtual void ValidateIntranodeTransmissionConfiguration();
    };
}
