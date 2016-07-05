/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IReport.h"
#include <list>
#include <vector>
#include "suids.hpp"

namespace Kernel
{
    class ReportPolioVirusPopulation: public BaseReport
    {
    public:
        ReportPolioVirusPopulation();
        virtual ~ReportPolioVirusPopulation() { }

        virtual void Initialize( unsigned int nrmSize ) override;
        virtual void Finalize() override;

        virtual void LogNodeData( Kernel::INodeContext * pNC ) override;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override { return false ; };
        virtual void LogIndividualData( Kernel::IIndividualHuman* individual ) override;
        virtual void BeginTimestep() override;
        virtual void EndTimestep( float currentTime, float dt ) override;

        virtual void Reduce() override;

        virtual std::string GetReportName() const override;

    private:
        std::ofstream VirusPopulation;
    
        typedef std::map<Kernel::suids::suid_data_t, std::stringstream*> node_to_stringstream_map_t;
        node_to_stringstream_map_t node_to_stats_map;

        void Reduce_Internal();
    };
}