/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "SimulationVector.h"
#include "ReportVector.h"
#include "SpatialReportVector.h"
#include "VectorSpeciesReport.h"
#include "NodeVector.h"
#include "IndividualVector.h"
#include "Sugar.h"
#include "Vector.h"
#include "SimulationConfig.h"
#include "JsonRawWriter.h"
#include "JsonRawReader.h"

#include <chrono>
typedef std::chrono::high_resolution_clock _clock;

static const char * _module = "SimulationVector";

namespace Kernel
{
    // QI stuff in case we want to use it more extensively
    BEGIN_QUERY_INTERFACE_DERIVED(SimulationVector, Simulation)
        HANDLE_INTERFACE(IVectorSimulationContext)
    END_QUERY_INTERFACE_DERIVED(SimulationVector, Simulation)

    SimulationVector::SimulationVector()
        : Kernel::Simulation()
        , drugdefaultcost(1.0f)
        , vaccinedefaultcost(DEFAULT_VACCINE_COST)
    {
        LOG_DEBUG( "SimulationVector ctor\n" );

        // The following arrays hold the total interventions distributed by intervention and distribution detail. 
        // This allows different costing models to be applied to study the effect of different distribution modes
        // the overall way the array is divided up is a bit difficult to parse out of this quickly, but this section will be redone in the next iteration for easier customization
        for (int i = 0; i < BEDNET_ARRAY_LENGTH; i++)
        {
            //type of net
            if (i < BEDNET_ARRAY_LENGTH / 4)
                netdefaultcost[i] = DEFAULT_BARRIER_COST;
            else if (i < BEDNET_ARRAY_LENGTH / 2 && i >= BEDNET_ARRAY_LENGTH / 4)
                netdefaultcost[i] = DEFAULT_ITN_COST;
            else if (i < 3 * BEDNET_ARRAY_LENGTH / 4 && i >= BEDNET_ARRAY_LENGTH / 2)
                netdefaultcost[i] = DEFAULT_LLIN_COST;
            else
                netdefaultcost[i] = DEFAULT_RETREATMENT_COST;

            //cost to user/campaign
            // TODO Those magic numbers you see before you today, you will never see again, but will be removed in distribution refactor in August 2011
            if (int(i / 64) == 1 || int(i / 64) == 4 || int(i / 64) == 7 || int(i / 64) == 10)
                netdefaultcost[i] *= 0.75;
            else if (int(i / 64) == 2 || int(i / 64) == 5 || int(i / 64) == 8 || int(i / 64) == 11)
                netdefaultcost[i] *= 0.1f;

            //delivery type
            if (int(i / 16) % 4 == 0)
                netdefaultcost[i] += 0.50; //sentinel
            else if (int(i / 16) % 4 == 1)
                netdefaultcost[i] += 1.00; //catchup without other campaign to share costs
            else if (int(i / 16) % 4 == 2)
                netdefaultcost[i] += 2.00; //door-to-door
            else if (int(i / 16) % 4 == 3)
                netdefaultcost[i] += 4.00; //door-to-door with verification

            //urban vs rural
            if (int(i / 2) % 2 == 0)
                netdefaultcost[i] += 0.17f; //urban
            else
                netdefaultcost[i] += 0.33f;   //rural
        }

        for (int i = 0; i < HOUSINGMOD_ARRAY_LENGTH; i++)
        {
            if (i < 4)
                housingmoddefaultcost[i] = DEFAULT_IRS_COST;
            else if (i < 8 && i >= 4)
                housingmoddefaultcost[i] = DEFAULT_SCREENING_COST;
            else
                housingmoddefaultcost[i] = DEFAULT_IRS_COST + DEFAULT_SCREENING_COST;
            if (int(i / 2) % 2 == 0)
                netdefaultcost[i] += 0.50; //urban
            else
                netdefaultcost[i] += 1.00; //rural
        }

        for (int i = 0; i < AWARENESS_ARRAY_LENGTH; i++)
        {
            awarenessdefaultcost[i] = 1.0;
        }
        reportClassCreator = ReportVector::CreateReport;
        spatialReportClassCreator = SpatialReportVector::CreateReport;
    }

    void SimulationVector::Initialize(const ::Configuration *config)
    {
        Simulation::Initialize(config);
        IndividualHumanVector fakeHuman;
        LOG_INFO( "Calling Configure on fakeHumanVector\n" );
        fakeHuman.Configure( config );
    }

    SimulationVector *SimulationVector::CreateSimulation()
    {
        return _new_ SimulationVector();
    }

    SimulationVector *SimulationVector::CreateSimulation(const ::Configuration *config)
    {
        SimulationVector *newsimulation = _new_ SimulationVector();
        if (newsimulation)
        {
            // This sequence is important: first
            // Creation-->Initialization-->Validation
            newsimulation->Initialize(config);
            if(!ValidateConfiguration(config))
            {
                delete newsimulation;
                newsimulation = nullptr;
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "VECTOR_SIM requested with invalid configuration." );
            }
        }

        return newsimulation;
    }

    bool SimulationVector::ValidateConfiguration(const ::Configuration *config)
    {
        bool validConfiguration = Kernel::Simulation::ValidateConfiguration(config);

        if (GET_CONFIGURABLE(SimulationConfig)->heterogeneous_intranode_transmission_enabled)
        {
            throw IncoherentConfigurationException(__FILE__, __LINE__, __FUNCTION__, "Sim_Type", "VECTOR_SIM", "Enable_Heterogeneous_Intranode_Transmission", "1" );
        }

        return validConfiguration;
    }

    SimulationVector::~SimulationVector()
    {
        // Node deletion handled by parent
        // No need to do any deletion for the flags
    }

    void SimulationVector::Reports_CreateBuiltIn()
    {
        // Do base-class behavior for creating one or more reporters
        Simulation::Reports_CreateBuiltIn();

        // If we're running a simulation with actual vectors, do VectorSpeciesReport as well
        if(!GET_CONFIGURABLE(SimulationConfig)->vector_species_names.empty())
            reports.push_back(VectorSpeciesReport::CreateReport( GET_CONFIGURABLE(SimulationConfig)->vector_species_names ));
        else
            LOG_INFO("Skipping VectorSpeciesReport; no vectors detected in simulation\n");
    }

    // called by demographic file Populate()
    void SimulationVector::addNewNodeFromDemographics(suids::suid node_suid, NodeDemographicsFactory *nodedemographics_factory, ClimateFactory *climate_factory)
    {
        NodeVector *node = NodeVector::CreateNode(this, node_suid);
        addNode_internal(node, nodedemographics_factory, climate_factory);
    }

    void SimulationVector::resolveMigration()
    {
//clorton        resolveMigrationInternal( typed_migration_queue_storage, migratingIndividualQueues );
//clorton        resolveMigrationInternal( typed_vector_migration_queue_storage, migratingVectorQueues );

        Simulation::resolveMigration(); // Take care of the humans

        std::vector< uint32_t > message_size_by_rank( EnvPtr->MPI.NumTasks );   // "buffers" for size of buffer messages
        std::list< MPI_Request > outbound_requests;     // requests for each outbound message
        std::list< JsonRawWriter* > outbound_messages;  // buffers for outbound messages

        for (int destination_rank = 0; destination_rank < EnvPtr->MPI.NumTasks; ++destination_rank)
        {
            if (destination_rank == EnvPtr->MPI.Rank)
            {
                // Don't bother to serialize locally
                for (auto individual : migratingVectorQueues[destination_rank])
                {
                    IMigrate* emigre = dynamic_cast<IMigrate*>(individual);
                    emigre->ImmigrateTo( nodes[emigre->GetMigrationDestination()] );
                }

//                auto writer = new JsonRawWriter();
//                IVectorCohort::serialize(*writer, migratingVectorQueues[destination_rank]);
//                for (auto& individual : migratingVectorQueues[destination_rank])
//                    individual->Recycle();
//                migratingVectorQueues[destination_rank].clear();
//                const char* buffer = ((IArchive*)writer)->GetBuffer();
//                auto reader = new JsonRawReader(buffer);
//                IVectorCohort::serialize(*reader, migratingVectorQueues[destination_rank]);
//                for (auto individual : migratingVectorQueues[destination_rank])
//                {
//                    IMigrate* immigrant = dynamic_cast<IMigrate*>(individual);
//                    immigrant->ImmigrateTo( nodes[immigrant->GetMigrationDestination()]);
//                }
//                delete reader;
//                delete writer;
            }
            else
            {
                auto writer = new JsonRawWriter();
//cout << "Emmigrating (" << destination_rank << "): " << migratingVectorQueues[destination_rank].size() << endl;
//auto t1 = _clock::now();
                IVectorCohort::serialize(*writer, migratingVectorQueues[destination_rank]);
//auto t2 = _clock::now();
                for (auto& individual : migratingVectorQueues[destination_rank])
                    individual->Recycle();  // delete individual
//auto t3 = _clock::now();

                uint32_t buffer_size = message_size_by_rank[destination_rank] = ((IArchive*)writer)->GetBufferSize();
                MPI_Request size_request;
                MPI_Isend(&message_size_by_rank[destination_rank], 1, MPI_UNSIGNED, destination_rank, 0, MPI_COMM_WORLD, &size_request);

                if (buffer_size > 0)
                {
                    const char* buffer = ((IArchive*)writer)->GetBuffer();
                    MPI_Request buffer_request;
                    MPI_Isend(const_cast<char*>(buffer), buffer_size, MPI_BYTE, destination_rank, 0, MPI_COMM_WORLD, &buffer_request);
                    outbound_requests.push_back(buffer_request);
                    outbound_messages.push_back(writer);
                }
//cout << "Serialize   (" << destination_rank << "): " << (t2 - t1).count() << endl;
//cout << "Delete      (" << destination_rank << "): " << (t3 - t2).count() << endl;
            }

            migratingVectorQueues[destination_rank].clear();
        }

        for (int source_rank = 0; source_rank < EnvPtr->MPI.NumTasks; ++source_rank)
        {
            if (source_rank == EnvPtr->MPI.Rank) continue;  // We don't use MPI to send individuals to ourselves.

            uint32_t size;
            MPI_Status status;
            MPI_Recv(&size, 1, MPI_UNSIGNED, source_rank, 0, MPI_COMM_WORLD, &status);

            if (size > 0)
            {
                unique_ptr<char[]> buffer(new char[size]);
                MPI_Status buffer_status;
                MPI_Recv(buffer.get(), size, MPI_BYTE, source_rank, 0, MPI_COMM_WORLD, &buffer_status);

//auto t3 = _clock::now();
                auto reader = make_shared<JsonRawReader>(buffer.get());
//auto t4 = _clock::now();
                IVectorCohort::serialize(*reader, migratingVectorQueues[source_rank]);
//auto t5 =_clock::now();
                for (auto individual : migratingVectorQueues[source_rank])
                {
                    IMigrate* immigrant = dynamic_cast<IMigrate*>(individual);
                    immigrant->ImmigrateTo( nodes[immigrant->GetMigrationDestination()] );
                }
//auto t6 = _clock::now();

//cout << "Parse       (" << source_rank << "): " << (t4 - t3).count() << endl;
//cout << "Deserialize (" << source_rank << "): " << (t5 - t4).count() << endl;
//cout << "Integrate   (" << source_rank << "): " << (t6 - t5).count() << endl;
//cout << "Immigrating (" << source_rank << "): " << migratingVectorQueues[source_rank].size() << endl;

                migratingVectorQueues[source_rank].clear();
          }
        }

        {   // Clean up from Isend(s)
            for (auto& request : outbound_requests)
            {
                MPI_Status status;
                MPI_Wait(&request, &status);
            }

            for (auto writer : outbound_messages)
            {
                delete writer;
            }
        }
    }

    void SimulationVector::PostMigratingVector(VectorCohort* ind)
    {
        // cast to VectorCohortIndividual
        // TBD: Get rid of cast, replace with QI. Not such a big deal at Simulation level
        VectorCohortIndividual* vci = static_cast<VectorCohortIndividual*>(ind);

        // put in queue by species and node rank
        migratingVectorQueues[nodeRankMap.GetRankFromNodeSuid(vci->GetMigrationDestination())].push_back(vci);
    }

    void SimulationVector::setupMigrationQueues()
    {
        Simulation::setupMigrationQueues();
        migratingVectorQueues.resize(EnvPtr->MPI.NumTasks);
    }

    ISimulationContext *SimulationVector::GetContextPointer()
    {
        return (ISimulationContext*)this;
    }

}

#if USE_BOOST_SERIALIZATION
BOOST_CLASS_EXPORT(Kernel::SimulationVector)
namespace Kernel {
    template<class Archive>
    void serialize(Archive & ar, SimulationVector& sim, const unsigned int  file_version )
    {
        static const char * _module = "SimulationVector";
        LOG_DEBUG("(De)serializing SimulationVector\n");

        // Register derived types (um these aren't derived...)
        ar.template register_type<Kernel::NodeVector>();
        ar.template register_type<Kernel::SusceptibilityVector>();
        ar.template register_type<Kernel::VectorCohort>();

        ar & sim.vaccinedefaultcost;
        ar & sim.housingmoddefaultcost;
        ar & sim.awarenessdefaultcost;
        ar & sim.netdefaultcost;

        // Serialize fields
        ar & sim.migratingVectorQueues;

        // Serialize base class
        ar & boost::serialization::base_object<Kernel::Simulation>(sim);

    }
}
#endif

