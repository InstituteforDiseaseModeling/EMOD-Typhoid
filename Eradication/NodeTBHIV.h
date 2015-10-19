/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "stdafx.h"

#include "NodeTB.h"
#include "IndividualTB.h" // for serialization junk


namespace Kernel
{
    class SpatialReportTB;
    class ReportTBHIV;

    class NodeTBHIV : public NodeTB
    {
        // TODO: Get rid of friending and provide accessors.
        //friend class ::ReportTBHIV;
        //friend class SpatialReportTBHIV;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        virtual ~NodeTBHIV(void);
        static NodeTBHIV *CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid);
        bool Configure( const Configuration* config );

        //virtual void SetupIntranodeTransmission();

        //virtual void SetNewInfectionState(InfectionStateChange::_enum inf_state_change, IndividualHuman *ih);

    protected:
        NodeTBHIV();
        NodeTBHIV(ISimulationContext *_parent_sim, suids::suid node_suid);

        //void Initialize();

        // Factory methods
        virtual IndividualHuman *createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty);

        //const SimulationConfig* params();

    private:
#if USE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, NodeTBHIV& node, const unsigned int  file_version );
#endif
    };
}
