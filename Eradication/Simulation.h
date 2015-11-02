/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

// test removal #include <functional>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include "suids.hpp"

#include "BoostLibWrapper.h"
#include "Contexts.h"
#include "Environment.h"
#include "IdmDateTime.h"
#include "Individual.h"
#include "Infection.h"
#include "ISimulation.h"
#include "Node.h"
#include "NodeRankMap.h"
#include "IReport.h"
// test removal #include "Susceptibility.h"
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
        virtual void PostMigratingIndividualHuman(IndividualHuman *i) override;

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
        boost::mpi::request sendHuman(IndividualHuman *ind_human, int dest_rank);
        IndividualHuman*    receiveHuman(int src_rank);
        virtual void resolveMigration(); // derived classes override this...

        // Campaign input file parsing
        virtual void   loadCampaignFromFile( const std::string& campaign_filename );

        virtual void notifyNewNodeObservers(INodeContext*);

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        // Nodes
        typedef std::map< suids::suid, Node* > NodeMap_t; // TODO: change to unordered_map for better asymptotic performance
        typedef NodeMap_t::value_type NodeMapEntry_t;
        NodeMap_t nodes;
        NodeRankMap nodeRankMap;

        std::vector<INodeEventContext*> node_event_context_list ;

        typedef boost::bimap<uint32_t, suids::suid> nodeid_suid_map_t;
        typedef nodeid_suid_map_t::value_type nodeid_suid_pair;
        nodeid_suid_map_t nodeid_suid_map;

        struct map_merge;

        // Migration
// clorton        std::vector< std::vector< IMigrate* > > migratingIndividualQueues;
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
        bool enable_default_report ;
        bool enable_event_report;
        std::string campaign_filename;
        std::string loadbalance_filename;
        int Run_Number;

        NodeDemographicsFactory* demographics_factory;
#pragma warning( pop )
    protected:

        //------------------------------------------------------------------------------------
        // N.B. Entire implementation of "resolveMigrationInternal" appears here in header file
        //      as a (necessary?) step towards disease-specific DLLs (because of the template)
        //------------------------------------------------------------------------------------

        void doJsonDeserializeReceive();

// clorton         // Helper class to minimize code duplication in derived classes
// clorton         template <class IndividualT>
// clorton         class TypedPrivateMigrationQueueStorage
// clorton         {
// clorton         public:
// clorton             virtual ~TypedPrivateMigrationQueueStorage() {}
// clorton 
// clorton             std::vector< IndividualT* > receive_queue;
// clorton             std::vector< std::vector< IndividualT* > > send_queue;
// clorton 
// clorton             TypedPrivateMigrationQueueStorage() { init(); }
// clorton 
// clorton             /* TEST virtual */ void init()
// clorton             {
// clorton                 send_queue.resize(EnvPtr->MPI.NumTasks);
// clorton                 receive_queue.resize(EnvPtr->MPI.NumTasks);
// clorton             }
// clorton         };
// clorton 
// clorton         // ...and call this with the individual types they desire along with the appropriately typed queue storage classes
// clorton         template <class IndividualT> 
// clorton         void resolveMigrationInternal( 
// clorton             TypedPrivateMigrationQueueStorage<IndividualT> &typed_migration_queue_storage,
// clorton             std::vector< std::vector< IMigrate* > > &migrating_queues
// clorton             )
// clorton         {
// clorton             // relatively inefficient but straightforward sync step
// clorton 
// clorton             // TODO: investigate allgather (or whatever) to sync up numbers of individuals being sent and received
// clorton 
// clorton             // Create a vector of individual counts to be sent. This is necessary because
// clorton             // I cannot reuse the same memory location in subsequent calls to MPI_Isend
// clorton             // (and MPI_Bsend incurs even uglier syntax)
// clorton             // (but still hack-y and possibly slower than it needs to be)
// clorton 
// clorton             individual_count_send_buffers.resize(EnvPtr->MPI.NumTasks); // ensure cached send count buffer has enough storage
// clorton 
// clorton             for (int dest_rank = 0; dest_rank < EnvPtr->MPI.NumTasks; dest_rank++)
// clorton             {
// clorton                 if (dest_rank == EnvPtr->MPI.Rank) 
// clorton                 {
// clorton                     // distribute individuals in the queue locally 
// clorton                     for (auto iterator = migrating_queues[dest_rank].rbegin(); iterator != migrating_queues[dest_rank].rend(); ++iterator)
// clorton                     {
// clorton                         auto migrant = *iterator;
// clorton                         migrant->ImmigrateTo( nodes[migrant->GetMigrationDestination()] );
// clorton                     }
// clorton                     migrating_queues[dest_rank].clear(); // clear the queue of locally migrating individuals so they aren't cleaned up later
// clorton                 }
// clorton                 else
// clorton                 {
// clorton                     // send number of individuals going to this processor in a message. possibly not most efficient communication scheme
// clorton 
// clorton                     MPI_Request request;
// clorton /* clorton
// clorton static const char * _module = "Simulation";
// clorton LOG_ERR_F("Sending %d individuals to rank %d.\n", int(migrating_queues[dest_rank].size()), dest_rank);
// clorton fflush(stdout);
// clorton clorton */
// clorton                     individual_count_send_buffers[dest_rank] = int(migrating_queues[dest_rank].size()); 
// clorton                     MPI_Isend(&individual_count_send_buffers[dest_rank], 1, MPI_INT, dest_rank, 0, MPI_COMM_WORLD, &request); 
// clorton                     pending_sends_plain.push_back(request);
// clorton 
// clorton                     // Use of Isend here presents a problem because I can't wait for the matching receive to be posted. 
// clorton                     // Provide different memory locations for each individual count. These values must remain valid until the barrier!
// clorton 
// clorton                     if (individual_count_send_buffers[dest_rank] > 0) 
// clorton                     {
// clorton                         //if (ENABLE_DEBUG_MPI_TIMING)
// clorton                         //    LOG_INFO_F("syncDistributedState(): Rank %d sending %d individuals to %d.\n", EnvPtr->MPI.Rank, individual_count_send_buffers[dest_rank], dest_rank);
// clorton 
// clorton                         // apparently I cannot send my array without casting the pointers in it first.
// clorton                         // utilize some cached class local storage to accomplish this
// clorton                         typed_migration_queue_storage.send_queue[dest_rank].resize(migrating_queues[dest_rank].size());
// clorton                         for (int k = 0; k < typed_migration_queue_storage.send_queue[dest_rank].size(); k++)
// clorton                         {
// clorton                             // static_cast appears crucial for the object to be sent whole and unperturbed. not sure why, I thought the pointer to the derived class started at the same offset as the base. but it appears not
// clorton                             typed_migration_queue_storage.send_queue[dest_rank][k] = static_cast<IndividualT*>(migrating_queues[dest_rank][k]);
// clorton                         }
// clorton 
// clorton #if (USE_BOOST_SERIALIZATION || USE_BOOST_MPI) && !USE_JSON_MPI
// clorton                         pending_sends.push_back(
// clorton                             EnvPtr->MPI.World->isend(dest_rank, 0, typed_migration_queue_storage.send_queue[dest_rank])
// clorton                             );
// clorton                         //LOG_DEBUG_F("sending %d individuals to %d.\n", individual_count_send_buffers[dest_rank], dest_rank);
// clorton 
// clorton                     }
// clorton                 }
// clorton             }
// clorton             // Done sending
// clorton /* clorton
// clorton LOG_ERR("Done sending migrating individuals.\n");
// clorton fflush(stdout);
// clorton clorton */
// clorton 
// clorton #if (USE_BOOST_SERIALIZATION || USE_BOOST_MPI) && !USE_JSON_MPI
// clorton             
// clorton             for (int src_rank = 0; src_rank < EnvPtr->MPI.NumTasks; src_rank++)
// clorton             {
// clorton                 if (src_rank == EnvPtr->MPI.Rank) continue; // we'll never receive a message from our own process
// clorton 
// clorton                 int receiving_humans = 0;
// clorton                 MPI_Status status;
// clorton                 MPI_Request request;
// clorton 
// clorton                 // Non-blocking receive
// clorton                 MPI_Irecv(&receiving_humans, 1, MPI_INT, src_rank, 0, MPI_COMM_WORLD, &request); // receive number of humans I need to receive
// clorton                 
// clorton                 // After issue the receiving command for number of humans, synchronous wait until the last MPI request (MPI_Irecv) to complete
// clorton                 MPI_Wait(&request, &status);
// clorton                 
// clorton                 //LOG_DEBUG_F("receiving_humans = %d from src=%d\n", receiving_humans, src_rank);
// clorton /* clorton
// clorton LOG_ERR_F("receiving_humans = %d from src=%d\n", receiving_humans, src_rank);
// clorton fflush(stdout);
// clorton clorton */
// clorton                 if (receiving_humans > 0) 
// clorton                 {
// clorton                     //if (ENABLE_DEBUG_MPI_TIMING)
// clorton                     //    LOG_INFO_F("syncDistributedState(): Rank %d receiving %d individuals from %d.\n", EnvPtr->MPI.Rank, receiving_humans, src_rank);
// clorton 
// clorton                     // receive the whole list structure
// clorton                     typed_migration_queue_storage.receive_queue.resize(receiving_humans);
// clorton 
// clorton                     boost::mpi::request recv_request = EnvPtr->MPI.World->irecv(src_rank, 0, typed_migration_queue_storage.receive_queue); 
// clorton 
// clorton                     recv_request.wait();
// clorton                     for (auto migrant : typed_migration_queue_storage.receive_queue)
// clorton                     {
// clorton                         migrant->ImmigrateTo( nodes[migrant->GetMigrationDestination()] );
// clorton                         // TODO: error handling if nodes[destination] not present on this rank
// clorton                     }
// clorton 
// clorton                     typed_migration_queue_storage.receive_queue.clear();
// clorton                     //std::cout << "migrating_humans = " << receiving_humans << std::endl;
// clorton                 }
// clorton             }
// clorton 
// clorton             // Cleanup phase
// clorton             // wait for any still pending sends, then free memory for individuals that were sent
// clorton             boost::mpi::wait_all(pending_sends.begin(), pending_sends.end());
// clorton             for (auto request : pending_sends_plain)    // important - gotta free em all!
// clorton             {
// clorton                 MPI_Status status;
// clorton                 MPI_Wait(&request, &status);
// clorton             }
// clorton 
// clorton             for (int dest_rank = 0; dest_rank < EnvPtr->MPI.NumTasks; dest_rank++)
// clorton             {
// clorton                 for (auto migrant : migrating_queues[dest_rank])
// clorton                 {
// clorton                     delete migrant;
// clorton                 }
// clorton 
// clorton                 migrating_queues[dest_rank].clear();
// clorton                 typed_migration_queue_storage.send_queue[dest_rank].clear();
// clorton             }
// clorton 
// clorton             pending_sends.clear();
// clorton             pending_sends_plain.clear();
// clorton 
// clorton             EnvPtr->MPI.World->barrier();
// clorton         }

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        map<void*, Kernel::ISimulation::callback_t> new_node_observers;
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
        void Reports_LogNodeData( Node* n );
        void PrintTimeAndPopulation();

        // Handling of passing "contexts" down to nodes, individuals, etc.
        virtual ISimulationContext *GetContextPointer();
        virtual void PropagateContextToDependents();

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        // non-persistent, cached memory for optimizations for resolveMigration
        std::vector<boost::mpi::request >    pending_sends;
        std::vector<MPI_Request         >    pending_sends_plain;

        // Create a vector of individual counts to be sent. 
        // This is necessary because I cannot reuse the same memory location
        // in subsequent calls to MPI_Isend (and MPI_Bsend incurs even uglier syntax) 
        // (but still hack-y and possibly slower than it needs to be)
        std::vector<int> individual_count_send_buffers;

// clorton        // Derived classes need to implement their own member of this type
// clorton        // Its constructor will be called from the simulation constructor and all will be happy.
// clorton        TypedPrivateMigrationQueueStorage<IndividualHuman> typed_migration_queue_storage; 
#pragma warning( pop )

#if USE_BOOST_SERIALIZATION
    private: // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, Simulation &sim, const unsigned int /* file_version */);
#endif
    };
}

#if USE_BOOST_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Kernel::ISimulationContext)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Kernel::SimulationEventContextHost) // ???
#endif
