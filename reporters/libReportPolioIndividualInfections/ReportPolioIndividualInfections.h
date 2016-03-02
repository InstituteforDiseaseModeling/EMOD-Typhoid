/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <map>
#include "IReport.h"
#include "suids.hpp"

namespace Kernel
{
    class ReportPolioIndividualInfections : public BaseReport
    {
    public:
        ReportPolioIndividualInfections();
        virtual ~ReportPolioIndividualInfections() { }

        virtual void Initialize( unsigned int nrmSize ) override;
        virtual void Finalize() override;

        virtual void LogNodeData( Kernel::INodeContext * pNC ) override;
        virtual bool IsCollectingIndividualData( float currentTime, float dt ) const override { return true ; } ;
        virtual void LogIndividualData( Kernel::IIndividualHuman* individual ) override;
        virtual void BeginTimestep() override;
        virtual void EndTimestep( float currentTime, float dt ) override;

        virtual void Reduce() override;

        virtual std::string GetReportName() const override;

    private:
        std::ofstream Infections;   // Only rank 0 will create an Infections file

        typedef std::map<Kernel::suids::suid_data_t, std::stringstream*> node_to_stringstream_map_t;
        node_to_stringstream_map_t node_to_infections_map;
        node_to_stringstream_map_t node_to_stats_map;

        void Reduce_Internal();
    };
}