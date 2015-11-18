/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <fstream>

#include "NodeRankMap.h"
#include "Log.h"
#include "FileSystem.h"
#include "JsonRawWriter.h"
#include "JsonRawReader.h"

static const char * _module = "NodeRankMap";
namespace Kernel {

    bool LegacyFileInitialLoadBalanceScheme::Initialize( std::string loadbalancefilename, uint32_t expected_num_nodes )
    {
        bool initialized = false;

        std::ifstream loadbalancefile;
        if(loadbalancefilename.length()) 
        {
            if( FileSystem::FileExists( loadbalancefilename ) )
            {
                loadbalancefile.open(loadbalancefilename, std::ios::binary);
            }
        }

        if(loadbalancefile.is_open()) 
        {
            loadbalancefile.seekg(0, std::ios::end);
            int filelen = int(loadbalancefile.tellg());
            int expected_size = sizeof(uint32_t) + (expected_num_nodes * (sizeof(uint32_t) + sizeof(float)));

            if(filelen == expected_size)
            {
                uint32_t num_nodes;
                loadbalancefile.seekg(0, std::ios::beg);
                loadbalancefile.read((char*)&num_nodes, 1*sizeof(num_nodes));

                if(num_nodes == expected_num_nodes)
                {
                    LOG_INFO_F("Opened %s, reading balancing data for %d nodes...\n", loadbalancefilename.c_str(), num_nodes);

                    std::vector<uint32_t> nodeids(num_nodes);
                    std::vector<float> balance_scale(num_nodes);

                    loadbalancefile.read((char*)&nodeids[0], (num_nodes)*sizeof(uint32_t));
                    loadbalancefile.read((char*)&balance_scale[0], (num_nodes)*sizeof(float));

                    for(uint32_t i = 0; i < num_nodes; i++)
                    {
                        initialNodeRankMapping[nodeids[i]] = int(EnvPtr->MPI.NumTasks*balance_scale[i]);
                    }
                    LOG_INFO("Static initial load balancing scheme initialized.\n");

                    initialized = true;
                }
                else
                {
                    LOG_WARN_F( "Malformed load-balancing file: %s.  Node-count specified in file (%d) doesn't match expected number of nodes (%d)\n", loadbalancefilename.c_str(), num_nodes, expected_num_nodes );
                }
            }
            else
            {
                LOG_WARN_F( "Problem with load-balancing file: %s.  File-size (%d) doesn't match expected size (%d) given number of nodes in demographics file\n", loadbalancefilename.c_str(), filelen, expected_size );
            }

            loadbalancefile.close();
        }
        else
        {
            LOG_WARN_F( "Failed to open load-balancing file: %s\n", loadbalancefilename.c_str() );
        }

        return initialized;
    }


    std::string NodeRankMap::ToString()
    {
        std::stringstream strstr;
        strstr << "{ NodeRankMap:" << std::endl;
        for (auto& entry : rankMap)
        {
            strstr << "[" << entry.first.data << "," << entry.second << "]" << std::endl;
        }
        strstr << "}" << std::endl;
        return strstr.str();
    }

    bool NodeRankMap::MergeMaps()
    {
        RankMap_t mergedMap;
        // Style note: exceptions used locally here to obey MPI semantics but also allow me to detect failure without an overly complex return-code mechanism
        try
        {
            LOG_DEBUG_F("%s\n", __FUNCTION__);
            mergedMap = rankMap;

            if (EnvPtr->MPI.NumTasks > 1)
            {
                auto json_writer = new JsonRawWriter();
                IArchive& writer = *static_cast<IArchive*>(json_writer);
                size_t count = rankMap.size();
                writer.startArray( count );
                LOG_VALID_F( "Serializing %d suid-rank map entries.\n", count );
                for (auto& entry : rankMap)
                {
                    writer.startObject();
                        uint32_t suid = entry.first.data;
                        writer.labelElement( "key" ) & suid;            // node.suid
                        writer.labelElement( "value" ) & entry.second;  // rank
                    writer.endObject();
                }
                writer.endArray();

                for (int rank = 0; rank < EnvPtr->MPI.NumTasks; ++rank)
                {
                    if (rank == EnvPtr->MPI.Rank)
                    {
                        const char* buffer = writer.GetBuffer();
                        uint32_t size = writer.GetBufferSize();
                        LOG_VALID_F( "Broadcasting serialized map (%d bytes)\n", size );
                        MPI_Bcast( (void*)&size, 1, MPI_INTEGER4, rank, MPI_COMM_WORLD );
                        MPI_Bcast( (void*)const_cast<char*>(buffer), size, MPI_BYTE, rank, MPI_COMM_WORLD );
                    }
                    else
                    {
                        uint32_t size;
                        MPI_Bcast( (void*)&size, 1, MPI_INTEGER4, rank, MPI_COMM_WORLD );
                        char* buffer = new char[size];
                        LOG_VALID_F( "Receiving map (%d bytes) from rank %d\n", size, rank );
                        MPI_Bcast( (void*)buffer, size, MPI_BYTE, rank, MPI_COMM_WORLD );
                        auto json_reader = new JsonRawReader( buffer );
                        IArchive& reader = *static_cast<IArchive*>(json_reader);
                        size_t count;
                        reader.startArray( count );
                            LOG_VALID_F( "Merging %d suid-rank map entries from rank %d\n", count, rank );
                            for (size_t i = 0; i < count; ++i)
                            {
                                suids::suid suid;
                                int32_t node_rank;
                                reader.startObject();
                                    reader.labelElement( "key" ) & suid.data;
                                    reader.labelElement( "value" ) & node_rank;
                                reader.endObject();
                                mergedMap[suid] = node_rank;
                            }
                        reader.endArray();
                        delete json_reader;
                        delete [] buffer;
                    }
                }

                delete json_writer;
            }
        }
        catch (std::exception &e)
        {
            LOG_ERR_F("MergeMaps() exception: %s\n",e.what());
            return false; // return failure; program must not continue if merge was invalid
        }

        rankMap = mergedMap;

        return true;
    }

    CheckerboardInitialLoadBalanceScheme::CheckerboardInitialLoadBalanceScheme()
    : num_ranked(0) { }

    int
    CheckerboardInitialLoadBalanceScheme::GetInitialRankFromNodeId(node_id_t node_id)
    { return (num_ranked++) % EnvPtr->MPI.NumTasks; }

    StripedInitialLoadBalanceScheme::StripedInitialLoadBalanceScheme()
        : num_nodes(0)
        , num_ranked(0)
    {}

    void StripedInitialLoadBalanceScheme::Initialize(uint32_t in_num_nodes) { num_nodes = in_num_nodes; }

    int
    StripedInitialLoadBalanceScheme::GetInitialRankFromNodeId(node_id_t node_id)
    { return int((float(num_ranked++) / num_nodes) * EnvPtr->MPI.NumTasks); }

    int LegacyFileInitialLoadBalanceScheme::GetInitialRankFromNodeId(node_id_t node_id)
    { return initialNodeRankMapping[node_id]; }

    NodeRankMap::NodeRankMap() : initialLoadBalanceScheme(nullptr) { }

    void NodeRankMap::SetInitialLoadBalanceScheme(IInitialLoadBalanceScheme *ilbs) { initialLoadBalanceScheme = ilbs; } 

    int NodeRankMap::GetRankFromNodeSuid(suids::suid node_id) { return rankMap[node_id]; } 

    size_t NodeRankMap::Size() { return rankMap.size(); }

    void NodeRankMap::Add(suids::suid node_suid, int rank) { rankMap.insert(RankMapEntry_t(node_suid, rank)); }

    const NodeRankMap::RankMap_t&
    NodeRankMap::GetRankMap() const { return rankMap; }

    int NodeRankMap::GetInitialRankFromNodeId(node_id_t node_id)
    {
        if (initialLoadBalanceScheme) { return initialLoadBalanceScheme->GetInitialRankFromNodeId(node_id); }
        else { return node_id % EnvPtr->MPI.NumTasks; }
    }

    const char*
    NodeRankMap::merge_duplicate_key_exception::what()
    const throw()
    { return "Duplicate key in map merge\n"; }

    NodeRankMap::RankMap_t
    NodeRankMap::map_merge::operator()(
        const RankMap_t& x,
        const RankMap_t& y
    )
    const
    {
        RankMap_t mergedMap;

        for (auto& entry : x)
        {
            if(!(mergedMap.insert(entry).second))
                throw(merge_duplicate_key_exception());
        }

        for (auto& entry : y)
        {
            if(!(mergedMap.insert(entry).second)) // .second is false if the key already existed
                throw(merge_duplicate_key_exception());
        }

        return mergedMap;
    }
}

#if 0
namespace Kernel
{
    template<typename Archive>
    void serialize(Archive & ar, NodeRankMap& nrm, const unsigned int file_version)
    {
        ar & nrm.rankMap;
    }
}
#endif
