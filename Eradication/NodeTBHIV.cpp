/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TB

#include "NodeTBHIV.h"
#include "TransmissionGroupsFactory.h" //for SetupIntranodeTransmission
#include "NodeEventContext.h" //for node level trigger
#include "IndividualCoinfection.h"

static const char* _module = "NodeTBHIV";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(NodeTBHIV, NodeTB)
    END_QUERY_INTERFACE_DERIVED(NodeTBHIV, NodeTB)


    NodeTBHIV::~NodeTBHIV(void) { }

    NodeTBHIV::NodeTBHIV() : NodeTB() { }

    NodeTBHIV::NodeTBHIV(ISimulationContext *_parent_sim, suids::suid node_suid) : NodeTB(_parent_sim, node_suid) { }

    NodeTBHIV *NodeTBHIV::CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid)
    {
        NodeTBHIV *newnode = _new_ NodeTBHIV(_parent_sim, node_suid);
        newnode->Initialize();

        return newnode;
    }

    bool NodeTBHIV::Configure( const Configuration* config )
    {
        return NodeTB::Configure( config );
    }

    /*void NodeTBHIV::Initialize()
    {
        NodeTB::Initialize();
    }*/

    IndividualHuman *NodeTBHIV::createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender,  float above_poverty)
    {
        return IndividualHumanCoinfection::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender, above_poverty);
    }
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(Kernel::NodeTBHIV)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, NodeTBHIV& node, const unsigned int  file_version )
    {
        // Register derived types // Really????
        ar.template register_type<IndividualHumanTB>();

        ar &boost::serialization::base_object<NodeTB>(node);    
    }
}
#endif

#endif // ENABLE_TB
