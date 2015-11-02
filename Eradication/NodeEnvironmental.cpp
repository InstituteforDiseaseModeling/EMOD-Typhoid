/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "NodeEnvironmental.h"
#include "IndividualEnvironmental.h"
#include "SusceptibilityEnvironmental.h"
#include "InfectionEnvironmental.h"
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

    IndividualHuman* NodeEnvironmental::createHuman( suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty )
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

    void NodeEnvironmental::SetupIntranodeTransmission()
    {
        transmissionGroups = TransmissionGroupsFactory::CreateNodeGroups(TransmissionGroupType::StrainAwareGroups);
        RouteToContagionDecayMap_t decayMap;
        LOG_DEBUG_F("Number of basestrains: %d\n", params()->number_basestrains);

        if( demographics.Contains( IP_KEY ) && params()->heterogeneous_intranode_transmission_enabled)
        {
            ValidateIntranodeTransmissionConfiguration();
            const NodeDemographics& properties = demographics[IP_KEY];
            for (int iProperty = 0; iProperty < properties.size(); iProperty++)
            {
                const NodeDemographics& property = properties[iProperty];
                if (property.Contains(TRANSMISSION_MATRIX_KEY))
                {
                    string propertyName = property[IP_NAME_KEY].AsString();
                    string routeName = property[TRANSMISSION_MATRIX_KEY][ROUTE_KEY].AsString();
                    std::transform(routeName.begin(), routeName.end(), routeName.begin(), ::tolower);
                    if (decayMap.find(routeName)==decayMap.end())
                    {
                        if (routeName =="contact")
                        {
                            decayMap[routeName] = 1.0f;
                            routes.push_back(routeName);
                        }
                        else if (routeName == "environmental")
                        {
                            decayMap[routeName] = node_contagion_decay_fraction;
                            routes.push_back(routeName);
                        }
                        else
                        {
                            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, std::string( "Found route " + routeName + ". For Environmental/polio sims, routes other than 'contact' and 'environmental' are not supported.").c_str());
                        }
                        LOG_DEBUG_F("HINT: Adding route %s.\n", routeName.c_str());
                    }

                    PropertyValueList_t valueList;
                    const NodeDemographics& propertyValues = property[IP_VALUES_KEY];
                    int valueCount = propertyValues.size();

                    const NodeDemographics& scalingMatrixRows = property[TRANSMISSION_MATRIX_KEY][TRANSMISSION_DATA_KEY];
                    ScalingMatrix_t scalingMatrix;

                    for (int iValue = 0; iValue < valueCount; iValue++)
                    {
                        valueList.push_back(propertyValues[iValue].AsString());
                        MatrixRow_t matrixRow;
                        const NodeDemographics& scalingMatrixRow = scalingMatrixRows[iValue];

                        for (int iSink = 0; iSink < valueCount; iSink++)
                        {
                            matrixRow.push_back((float)scalingMatrixRow[iSink].AsDouble());
                        }

                        scalingMatrix.push_back(matrixRow);
                    }
                    LOG_DEBUG_F("adding property [%s]:%s\n", propertyName.c_str(), routeName.c_str());
                    transmissionGroups->AddProperty(propertyName, valueList, scalingMatrix, routeName);
                }
                else //HINT is enabled, but no transmission matrix is detected
                {
                    string default_route = string("environmental");
                    float default_fraction = node_contagion_decay_fraction;
                    if (decayMap.find(default_route)==decayMap.end())
                    {
                        LOG_DEBUG("HINT on with no transmission matrix: Adding route 'environmental'.\n");
                        decayMap[default_route] = default_fraction;
                        routes.push_back(default_route);
                    }
                }
            }

        }
        else
        {
            //default scenario with no HINT
            LOG_DEBUG("non-HINT: Adding route 'environmental'.\n");
            decayMap[string("environmental")] = node_contagion_decay_fraction;
            routes.push_back(string("environmental"));
        }

        transmissionGroups->Build(decayMap, params()->number_basestrains, params()->number_substrains);
    }

    void NodeEnvironmental::ValidateIntranodeTransmissionConfiguration()
    {
        Node::ValidateIntranodeTransmissionConfiguration();
    }
}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(Kernel::NodeEnvironmental)
namespace Kernel {
    template <typename Archive>
    void serialize(Archive & ar, NodeEnvironmental& node, const unsigned int /* file_version */)
    {
        ar & node.contagion;
        ar & node.node_contagion_decay_fraction;
        //ar.template register_type<IndividualHumanEnvironmental>();
        ar & boost::serialization::base_object<Node>(node);
    }
}
#endif

#endif // ENABLE_POLIO
