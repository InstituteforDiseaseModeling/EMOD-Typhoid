/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include <map>
#include "BoostLibWrapper.h"
#include "suids.hpp"
#include "INodeContext.h"

namespace Kernel
{
    struct IInitialLoadBalanceScheme;

    /*
    TODO: 
    x leave node rank with map impl for now, hash_map to be implemented later if needed for performance
    */

    // base class for node->rank maps
    class NodeRankMap 
    {
    public:
        NodeRankMap();
        ~NodeRankMap();

        // NOTE: the initial scheme object is NOT serialized.  It is ONLY for initializing the node rank map the first time it is populated
        void SetInitialLoadBalanceScheme( IInitialLoadBalanceScheme *ilbs );

        int GetRankFromNodeSuid(suids::suid node_id);

        size_t Size();

        // node rank maps will be built up as nodes are added during populate()
        // then other ranks need to find out that our nodes do indeed live with us
        // therefore, we will send all the other nodes a copy of our map to be merged
        // merge maps on multiple processors
        bool MergeMaps();

        // this function encapsulates the initial mapping from node id on disk to rank.
        // although node ids should be simulation-unique we still track nodes by our own suid 
        // system. this may not ever be strictly advantageous but it might make it easier to, 
        // e.g., generate new nodes later in the simulation for example
        int GetInitialRankFromNodeId( ExternalNodeId_t node_id );

        void Add(suids::suid node_suid, int rank);

        std::string ToString();

        // hack: to let us get the complete list of nodes
        typedef std::map<suids::suid, int> RankMap_t;
        typedef std::pair<suids::suid, int> RankMapEntry_t;

        const RankMap_t& GetRankMap() const;

    private:
        IInitialLoadBalanceScheme *initialLoadBalanceScheme;

        RankMap_t rankMap;

        struct merge_duplicate_key_exception : public std::exception
        {
            virtual const char* what() const throw();
        };

        struct map_merge : public std::binary_function<RankMap_t, RankMap_t, RankMap_t>
        {
            RankMap_t operator()(const RankMap_t& x, const RankMap_t& y) const;
        };
    };
}
