/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IReport.h"
#include <list>
#include <vector>
#include "suids.hpp"

class ReportPolioVirusPopulation: public IReport
{
public:
    ReportPolioVirusPopulation();
    virtual ~ReportPolioVirusPopulation() { }

    virtual void Initialize( unsigned int nrmSize );
    virtual void Finalize();

    virtual void LogNodeData( Kernel::INodeContext * pNC );
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const { return false ; } ;
    virtual void LogIndividualData( Kernel::IndividualHuman * individual );
    virtual void BeginTimestep();
    virtual void EndTimestep( float currentTime, float dt );

    virtual void Reduce();

    virtual std::string GetReportName() const;

private:
    std::ofstream VirusPopulation;
    
    typedef std::map<Kernel::suids::suid_data_t, std::stringstream*> node_to_stringstream_map_t;
    node_to_stringstream_map_t node_to_stats_map;

    void Reduce_Internal();
};
