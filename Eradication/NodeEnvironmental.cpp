/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "NodeEnvironmental.h"
#include "IndividualEnvironmental.h"
#include "TransmissionGroupsFactory.h"
#include "SimulationConfig.h"

static const char * _module = "NodeEnvironmental";

namespace Kernel
{
   
    NodeEnvironmental::NodeEnvironmental()
    : Node()
    , contagion(0)
    {
    }

    NodeEnvironmental::NodeEnvironmental(ISimulationContext *_parent_sim, suids::suid node_suid)
    : Node(_parent_sim, node_suid)
    , contagion(0)
    {
    }

    NodeEnvironmental::~NodeEnvironmental(void)
    {
    }

    NodeEnvironmental *NodeEnvironmental::CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid)
    {
        NodeEnvironmental *newnode = _new_ NodeEnvironmental(_parent_sim, node_suid);
        newnode->Initialize();

        return newnode;
    }

    bool NodeEnvironmental::Configure( const Configuration* config )
    {
        initConfigTypeMap( "Node_Contagion_Decay_Rate", &node_contagion_decay_fraction, Node_Contagion_Decay_Rate_DESC_TEXT, 0.0f, 1.0f, 1.0f ); //this value represents *fraction* of contagion not carried over to the next time step, see EndUpdate() in transmissiongroups
        return Node::Configure( config );
    }

    IIndividualHuman* NodeEnvironmental::createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty )
    {
        return IndividualHumanEnvironmental::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender, above_poverty);
    }

    float NodeEnvironmental::getClimateInfectivityCorrection() const
    {
        // Environmental infectivity depends on rainfall.
        // TODO: make more configurable to accommodate different modalities:
        //       - high rainfall inducing prolonged flooding
        //       - high rainfall causing dilution and flushing vs. low rainfall causing concentration
        //       - etc.

        float correction = 1.0f;

        if ( localWeather == nullptr )
        {
            throw NullPointerException( __FILE__, __LINE__, __FUNCTION__, "localWeather", "Climate");
            //throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "climate_structure", "CLIMATE_OFF", "infectivity_scaling", "FUNCTION_OF_CLIMATE");
        }

        float rainfall = localWeather->accumulated_rainfall();

        // The following is a linear increase in infectivity above a threshold of 10mm of rainfall
        if ( rainfall > 0.01 )
        {
            correction += (rainfall - 0.01) / 0.01;
        }
        LOG_DEBUG_F( "Infectivity scale factor = %f at rainfall = %f.\n", correction, rainfall );

        return correction;
    }

    ITransmissionGroups* NodeEnvironmental::CreateTransmissionGroups()
    {
        return TransmissionGroupsFactory::CreateNodeGroups( TransmissionGroupType::StrainAwareGroups );
    }

    void NodeEnvironmental::AddDefaultRoute( RouteToContagionDecayMap_t& rDecayMap )
    {
        AddRoute( rDecayMap, "environmental" );
    }

    void NodeEnvironmental::AddRoute( RouteToContagionDecayMap_t& rDecayMap, const std::string& rRouteName )
    {
        if( rDecayMap.find( rRouteName ) == rDecayMap.end() )
        {
            float contagion_decay = 1.0;
            if( rRouteName =="contact" )
            {
                contagion_decay = 1.0;
            }
            else if( rRouteName == "environmental" )
            {
                contagion_decay = node_contagion_decay_fraction;
            }

            LOG_DEBUG_F("Adding route %s.\n", rRouteName.c_str());
            rDecayMap[ rRouteName ] = contagion_decay;
            routes.push_back( rRouteName );
        }
    }

    void NodeEnvironmental::BuildTransmissionRoutes( RouteToContagionDecayMap_t& rDecayMap )
    {
        LOG_DEBUG_F("Number of basestrains: %d\n", params()->number_basestrains);
        transmissionGroups->Build( rDecayMap, params()->number_basestrains, params()->number_substrains );
    }

    bool NodeEnvironmental::IsValidTransmissionRoute( string& transmissionRoute )
    {
        bool isValid = ((transmissionRoute == "contact") || (transmissionRoute == "environmental"));
        return isValid;
    }

    REGISTER_SERIALIZABLE(NodeEnvironmental);

    void NodeEnvironmental::serialize(IArchive& ar, NodeEnvironmental* obj)
    {
        Node::serialize(ar, obj);
        NodeEnvironmental& node = *obj;
        ar.labelElement("contagion") & node.contagion;
        ar.labelElement("node_contagion_decay_fraction") & node.node_contagion_decay_fraction;
    }
}

#if 0
namespace Kernel {
    template <typename Archive>
    void serialize(Archive & ar, NodeEnvironmental& node, const unsigned int /* file_version */)
    {
        ar & node.contagion;
        ar & node.node_contagion_decay_fraction;
        ar & boost::serialization::base_object<Node>(node);
    }
}
#endif

#endif // ENABLE_POLIO
