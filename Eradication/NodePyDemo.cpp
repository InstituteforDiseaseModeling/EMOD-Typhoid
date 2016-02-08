/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include <math.h>
#include <numeric> // for std::accumulate
#include <functional> // why not algorithm?
#include "Sugar.h"
#include "Exceptions.h"
#include "NodePyDemo.h"
#include "IndividualPyDemo.h"
#include "TransmissionGroupsFactory.h"
#include "SimulationConfig.h"
#include "Python.h"

using namespace Kernel;

static const char* _module = "NodePyDemo";

extern PyObject *
IdmPyInit(
    const char * python_script_name,
    const char * python_function_name
);

#define ENABLE_PYTHOID 1
namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(NodePyDemo, Node)
        HANDLE_INTERFACE(INodePyDemo)
    END_QUERY_INTERFACE_DERIVED(NodePyDemo, Node)


    NodePyDemo::NodePyDemo() : NodeEnvironmental() { }

    NodePyDemo::NodePyDemo(ISimulationContext *_parent_sim, suids::suid node_suid) : NodeEnvironmental(_parent_sim, node_suid)
    {
    }

    void NodePyDemo::Initialize()
    {
        NodeEnvironmental::Initialize();
    }

    bool NodePyDemo::Configure(
        const Configuration* config
    )
    {
        return NodeEnvironmental::Configure( config );
    }

    NodePyDemo *NodePyDemo::CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid)
    {
        NodePyDemo *newnode = _new_ NodePyDemo(_parent_sim, node_suid);
        newnode->Initialize();

        return newnode;
    }

    NodePyDemo::~NodePyDemo(void)
    {
    }

#define ROUTE_NAME_ENVIRONMENTAL "environmental"
#define ROUTE_NAME_CONTACT       "contact"
    void NodePyDemo::SetupIntranodeTransmission()
    {
        //transmissionGroups = TransmissionGroupsFactory::CreateNodeGroups( TransmissionGroupType::MultiRouteGroups );
        transmissionGroups = TransmissionGroupsFactory::CreateNodeGroups( TransmissionGroupType::StrainAwareGroups );
        RouteToContagionDecayMap_t decayMap;
        LOG_DEBUG_F("Number of basestrains: %d\n", GET_CONFIGURABLE(SimulationConfig)->number_basestrains);

        if( demographics.Contains( IP_KEY ) && GET_CONFIGURABLE(SimulationConfig)->heterogeneous_intranode_transmission_enabled)
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
                        if (routeName == ROUTE_NAME_CONTACT )
                        {
                            decayMap[routeName] = 1.0f;
                            routes.push_back(routeName);
                        }
                        else if (routeName == ROUTE_NAME_ENVIRONMENTAL )
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
            LOG_DEBUG("non-HINT: Adding route 'environmental' and 'contact'.\n");
            decayMap[string( ROUTE_NAME_ENVIRONMENTAL )] = node_contagion_decay_fraction;
            decayMap[string( ROUTE_NAME_CONTACT )] = 1.0;
            routes.push_back( ROUTE_NAME_ENVIRONMENTAL );
            routes.push_back(string( ROUTE_NAME_CONTACT ));
        }

        transmissionGroups->Build(decayMap, GET_CONFIGURABLE(SimulationConfig)->number_basestrains, GET_CONFIGURABLE(SimulationConfig)->number_substrains);
    }

    void NodePyDemo::resetNodeStateCounters(void)
    {
        // This is a chance to do a single call into Pythoid at start of timestep
#ifdef ENABLE_PYTHOID
        static auto pFunc = IdmPyInit( "dtk_typhoid_individual", "start_timestep" );
        if( pFunc )
        {
            PyObject_CallObject( pFunc, nullptr );
        }
#endif

        NodeEnvironmental::resetNodeStateCounters();
    }

    void NodePyDemo::updateNodeStateCounters(IndividualHuman *ih)
    {
        float mc_weight                = float(ih->GetMonteCarloWeight());
        IIndividualHumanPyDemo *tempind2 = NULL;
        if( ih->QueryInterface( GET_IID( IIndividualHumanPyDemo ), (void**)&tempind2 ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "tempind2", "IndividualHumanPyDemo", "IndividualHuman" );
        }

        NodeEnvironmental::updateNodeStateCounters(ih);
    }


    void NodePyDemo::finalizeNodeStateCounters(void)
    {
        NodeEnvironmental::finalizeNodeStateCounters();
       
    }

    void NodePyDemo::populateNewIndividualsFromDemographics(int count_new_individuals)
    {
        // Populate the initial population
        Node::populateNewIndividualsFromDemographics(count_new_individuals);
    }

    IndividualHuman *NodePyDemo::createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty)
    {
        return IndividualHumanPyDemo::CreateHuman(this, suid, monte_carlo_weight, initial_age, gender, above_poverty);
    }

    std::map< std::string, float >
    NodePyDemo::GetTotalContagion()
    const
    {
        std::map< std::string, float > returnThis;
        //auto routes = GetTransmissionRoutes();
        unsigned int route_idx = 0;
        for( auto & route: GetTransmissionRoutes() )
        {
            // how do we get membership? That's from an individual, but we are at node level here?????
            // Need to get proper mapping for route name, route idx, and group id. Just hacking it here.
            TransmissionGroupMembership_t membership;
            membership[ 1-route_idx ] = 0;
            route_idx++;
            auto contagion = transmissionGroups->GetTotalContagion(&membership);
            returnThis.insert( std::make_pair( route, contagion ) );
			///LOG_INFO_F("route and contagion %s, %f\n", route, contagion);
        }
        return returnThis;
    }

#if USE_BOOST_SERIALIZATION
    BOOST_CLASS_EXPORT(NodePyDemo)
    namespace Kernel {
        template<class Archive>
        void serialize(Archive & ar, NodePyDemo& node, const unsigned int /* file_version */)
        { 
            ar.template register_type<IndividualHumanPyDemo>();
            ar & boost::serialization::base_object<NodeEnvironmental>(node);
        }
    }
#endif

    NodePyDemoTest *
    NodePyDemoTest::CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid)
    {
        auto *newnode = _new_ NodePyDemoTest(_parent_sim, node_suid);
        newnode->Initialize();

        return newnode;
    }

    NodePyDemoTest::NodePyDemoTest(ISimulationContext *_parent_sim, suids::suid node_suid)
    {
        parent = _parent_sim;
        auto newPerson = configureAndAddNewIndividual(1.0F /*mc*/, 0 /*age*/, 0.0f /*prev*/, 0.5f /*gender*/); // N.B. temp_prevalence=0 without maternal_transmission flag
        for (auto pIndividual : individualHumans)
        {
             // Nothing to do at the moment.
        }
    }
}
#endif // ENABLE_POLIO
