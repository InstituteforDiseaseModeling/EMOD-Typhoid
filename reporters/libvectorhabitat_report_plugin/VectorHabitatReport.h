/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "BinnedReport.h"

namespace Kernel {

class VectorHabitatReport : public BinnedReport
{
public:
    VectorHabitatReport();
    virtual ~VectorHabitatReport();

    virtual void Initialize( unsigned int nrmSize );

    virtual void LogNodeData( Kernel::INodeContext * pNC );
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const { return false ; } ;
    virtual void LogIndividualData( Kernel::IIndividualHuman* individual );
    virtual void EndTimestep( float currentTime, float dt );

protected:
    virtual void initChannelBins();
    void clearChannelsBins();
    void Accumulate( const std::string& channel_name, const ChannelDataMap::channel_data_t& binned_data);
    virtual int calcBinIndex(const Kernel::IIndividualHuman* individual);

    typedef std::map< std::string, int > habitat_idx_map_t;
    habitat_idx_map_t species_habitat_idx_map;  // e.g. ("arabiensis:CONSTANT", 1)

    ChannelDataMap::channel_data_t current_habitat_capacity;
    ChannelDataMap::channel_data_t total_larva;
    ChannelDataMap::channel_data_t egg_crowding_factor;
    ChannelDataMap::channel_data_t local_larval_mortality;
    ChannelDataMap::channel_data_t artificial_larval_mortality;
    ChannelDataMap::channel_data_t local_larval_growth_mod;
};

}

