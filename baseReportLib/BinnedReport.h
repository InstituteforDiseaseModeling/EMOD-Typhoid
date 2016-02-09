/*****************************************************************************

Copyright (c) 2014 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once

#include "BaseChannelReport.h"

namespace json {
    class Element;
}

namespace Kernel {
    struct IJsonObjectAdapter;
}

class BinnedReport : public BaseChannelReport
{
public:
    static IReport* CreateReport();
    virtual ~BinnedReport();

    virtual void Initialize( unsigned int nrmSize );

    virtual void BeginTimestep();
    virtual void LogNodeData( Kernel::INodeContext * pNC );
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const { return true ; } ;
    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void EndTimestep( float currentTime, float dt );

    virtual void Finalize();

protected:
    BinnedReport();

    virtual void Accumulate(std::string channel_name, float bin_data[]);
    virtual void Accumulate(std::string channel_name, float value, int bin_index);
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map );
    virtual void postProcessAccumulatedData();

    virtual void initChannelBins();
    void clearChannelsBins();
    virtual int calcBinIndex(Kernel::IndividualHuman * individual);

    // TODO: should return-type be something generic (void* ?) so e.g. MATLAB plugin can follow this pattern?
    virtual json::Element formatChannelDataBins(const float data[], std::vector<int>& dims, int start_axis, int num_remaining_bins);
    void formatChannelDataBins(Kernel::IJsonObjectAdapter* pIJsonObj, const float data[], std::vector<int>& dims, int start_axis, int num_remaining_bins);

    // BaseChannelReport
    virtual void SetAugmentor( IChannelDataMapOutputAugmentor* pAugmentor ) { p_output_augmentor = pAugmentor; };

    // general sub-channel binning
    int num_timesteps;
    int num_axes;
    std::vector<std::string> axis_labels;
    std::vector<int> num_bins_per_axis;
    int num_total_bins;
    std::vector<std::vector<float> > values_per_axis;
    std::vector<std::vector<std::string> > friendly_names_per_axis;

    //labels (these are in the header instead of the cpp file so they can be inherited by disease specific binned report
    static const char * _pop_label;
    static const char * _infected_label;
    static const char * _new_infections_label;
    static const char * _disease_deaths_label;
    // channels specific to this particular report-type
    float *population_bins;
    float *infected_bins;
    float *new_infections_bins;
    float *disease_deaths_bins;
    IChannelDataMapOutputAugmentor* p_output_augmentor ;
    int _num_age_bins;
    int _num_bins_per_axis[2];
    float _age_bin_upper_values[100];
    //std::string _age_bin_friendly_names[100];
    std::vector<std::string> _age_bin_friendly_names;
                                  
};
