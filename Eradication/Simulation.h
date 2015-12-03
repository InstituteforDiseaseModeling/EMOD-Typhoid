/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <list>
#include <map>
#include <unordered_map>
#include "suids.hpp"

#include "BoostLibWrapper.h"
#include "Contexts.h"
#include "IdmDateTime.h"
#include "Individual.h"
#include "Infection.h"
#include "ISimulation.h"
#include "Node.h"
#include "NodeRankMap.h"
#include "IReport.h"
#include "Configure.h"
#include "IdmApi.h"

class RANDOMBASE;

#define ENABLE_DEBUG_MPI_TIMING 0  // TODO: could make this an environment setting so we don't have to recompile

namespace Kernel
{
    class  CampaignEvent;
    struct IEventCoordinator;
    struct SimulationEventContext;
    class  SimulationEventContextHost;

    typedef uint32_t node_id_t;

    class IDMAPI Simulation : public ISimulation, public ISimulationContext, public JsonConfigurable
    {
        GET_SCHEMA_STATIC_WRAPPER(Simulation)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:

        static Simulation *CreateSimulation();  // for serialization
        static Simulation *CreateSimulation(const ::Configuration *config);
        virtual ~Simulation();

        virtual bool Configure( const ::Configuration *json ) override;

        // IGlobalContext interfaces
        virtual const SimulationConfig* GetSimulationConfigObj() const override;
        virtual const IInterventionFactory* GetInterventionFactory() const override;

        // ISimulation methods
        virtual bool  Populate() override;
        virtual void  Update(float dt) override;
        virtual int   GetSimulationTimestep() const override;
        virtual IdmDateTime GetSimulationTime() const override;
        virtual void  RegisterNewNodeObserver(void* id, Kernel::ISimulation::callback_t observer) override;
        virtual void  UnregisterNewNodeObserver(void* id) override;
        virtual void  WriteReportsData() override;

        virtual const DemographicsContext* GetDemographicsContext() const override;

        // Migration
        virtual void PostMigratingIndividualHuman(IIndividualHuman *i) override;

        // Unique ID services
        virtual suids::suid GetNextNodeSuid() override;
        virtual suids::suid GetNextIndividualHumanSuid() override;
        virtual suids::suid GetNextInfectionSuid() override;

        // Random number handling
        virtual RANDOMBASE* GetRng() override;
        void ResetRng(); // resets random seed to a new value. necessary when branching from a common state

        // Reporting
        virtual std::vector<IReport*>& GetReports() override;
        virtual std::vector<IReport*>& GetReportsNeedingIndividualData() override;


    protected:

        Simulation();
        virtual void Initialize();  // for serialization
        virtual void Initialize(const ::Configuration *config);

        static bool ValidateConfiguration(const ::Configuration *config);

        virtual void Reports_CreateBuiltIn();
        virtual void Reports_CreateCustom();

        // Initialization
        virtual void setupEventContextHost();
        virtual void setupMigrationQueues();
        void setupRng();
        void setParams( const ::Configuration *config );
        void initSimulationState();

        // TODO: this is only here temporarily... should really go in a MigrationFactory class or 
        // something similar to ClimateFactory::ParseMetadataForFile() when migration gets refactored
// test removal        bool ParseMetadataForMigrationFile(std::string data_filepath, std::string idreference, hash_map<uint32_t, uint32_t> &node_offsets);

        // Node initialization
        int  populateFromDemographics(const char* campaign_filename, const char* loadbalance_filename); // creates nodes from demographics input file data
        virtual void addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory); // For derived Simulation classes to add correct node type
        void addNode_internal(Node *node, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory); // Helper to add Nodes
        int  getInitialRankFromNodeId(node_id_t node_id); // Needed in MPI implementation

        // Migration
        virtual void resolveMigration(); // derived classes override this...

        // Campaign input file parsing
        virtual void   loadCampaignFromFile( const std::string& campaign_filename );

        virtual void notifyNewNodeObservers(INodeContext*);

        uint32_t serializationMask;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        // Nodes
        typedef std::map< suids::suid, INodeContext* > NodeMap_t; // TODO: change to unordered_map for better asymptotic performance
        typedef NodeMap_t::value_type NodeMapEntry_t;
        NodeMap_t nodes;
        NodeRankMap nodeRankMap;

        std::vector<INodeEventContext*> node_event_context_list ;

        typedef boost::bimap<uint32_t, suids::suid> nodeid_suid_map_t;
        typedef nodeid_suid_map_t::value_type nodeid_suid_pair;
        nodeid_suid_map_t nodeid_suid_map;

        // Migration
        std::vector<std::vector<IIndividualHuman*>> migratingIndividualQueues;

        // Master copies of contained-class flags are maintained here so that they only get serialized once
        // TODO: deprecate and use SimulationConfig everywhere
        const SimulationConfig*     m_simConfigObj;
        const IInterventionFactory* m_interventionFactoryObj;
        const DemographicsContext *demographicsContext;

        // Simulation-unique ID generators for each type of child object that might exist in our system
        suids::distributed_generator<Infection> infectionSuidGenerator;
        suids::distributed_generator<IndividualHuman> individualHumanSuidGenerator;
        suids::distributed_generator<Node> nodeSuidGenerator;

        // Input files
        std::string campaignFilename;
        std::string loadBalanceFilename;

        // RNG services
        RANDOMBASE* rng;

        // Reporting
        std::vector<IReport*> reports;                          // Reporter container
        std::vector<IReport*> individual_data_reports;          // subset of 'reports' vector

        typedef IReport* (*tReportClassCreator)();
        tReportClassCreator reportClassCreator;                 // Reporting class factory/creator function pointer.
        tReportClassCreator binnedReportClassCreator;
        tReportClassCreator spatialReportClassCreator;
        tReportClassCreator propertiesReportClassCreator;
        tReportClassCreator demographicsReportClassCreator;
        tReportClassCreator eventReportClassCreator;

        // Coordination of events for campaign intervention events
        std::list<IEventCoordinator*> event_coordinators;
        std::list<CampaignEvent*>     campaign_events;

        friend class SimulationEventContextHost;
        friend class Node;
        SimulationEventContextHost *event_context_host;

        // Parameters
        float Ind_Sample_Rate; // Fraction of individuals in each node to sample; can be modified for each community

        // Counters
        IdmDateTime currentTime;

        // JsonConfigurable variables
        RandomType::Enum                                    random_type;                                // RANDOM_TYPE
        SimType::Enum                                        sim_type;                                    // Sim_Type
        bool demographic_tracking;
        bool enable_spatial_output;
        bool enable_property_output;
        bool enable_default_report;
        bool enable_event_report;
        std::string campaign_filename;
        std::string loadbalance_filename;
        int Run_Number;

        NodeDemographicsFactory* demographics_factory;
#pragma warning( pop )
    protected:

        void MergeNodeIdSuidBimaps( nodeid_suid_map_t&, nodeid_suid_map_t& );

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        map<void*, Kernel::ISimulation::callback_t> new_node_observers;

        DECLARE_SERIALIZABLE(Simulation);
        static void serialize(IArchive&, NodeMap_t&);
#pragma warning( pop )

    private:
        typedef std::unordered_map< std::string, report_instantiator_function_t > ReportInstantiatorMap ;
        void Reports_ConfigureBuiltIn();
        void Reports_FindReportsCollectingIndividualData( float currentTime, float dt );
        Configuration* Reports_GetCustomReportConfiguration();
        void Reports_Instantiate( ReportInstantiatorMap& rReportInstantiatorMap );
        void Reports_UpdateEventRegistration( float _currentTime, float dt );
        void Reports_BeginTimestep();
        void Reports_EndTimestep( float _currentTime, float dt );
        void Reports_LogNodeData( INodeContext* n );
        void PrintTimeAndPopulation();

        // Handling of passing "contexts" down to nodes, individuals, etc.
        virtual ISimulationContext *GetContextPointer();
        virtual void PropagateContextToDependents();
    };
}
