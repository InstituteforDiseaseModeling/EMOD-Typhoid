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

class TajikReporter : public IReport
{
public:
    TajikReporter();
    virtual ~TajikReporter() { }

    virtual void Initialize( unsigned int nrmSize );
    virtual void Finalize();

    virtual void BeginTimestep();
    virtual void LogNodeData( Kernel::INodeContext * pNC );
    virtual bool IsCollectingIndividualData( float currentTime, float dt ) const { return false ; } ;
    virtual void LogIndividualData( Kernel::IIndividualHuman* individual );
    virtual void EndTimestep( float currentTime, float dt );

    virtual void Reduce();

    virtual std::string GetReportName() const;

protected:
    void WriteKmlData();

    typedef std::vector<float> tNode2DataMapVecType;
    typedef std::map< Kernel::suids::suid_data_t, tNode2DataMapVecType > tNode2DataMapVec;  // Node ID -> Vector of float
    typedef std::map< std::string, tNode2DataMapVec > tChannel2Node2DataMapVec; // Channel string -> map
    tChannel2Node2DataMapVec nodeChannelMapVec;

    typedef std::map< Kernel::suids::suid_data_t, float > tNode2DataMap;    // Node ID -> Single float
    typedef std::map< std::string, tNode2DataMap > tChannel2Node2DataMap;   // Channel string -> map
    tChannel2Node2DataMap nodeChannelMap;
};

