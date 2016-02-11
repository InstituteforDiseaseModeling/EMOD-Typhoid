/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once

#include "ReportEnvironmental.h"
#include "PyDemoDefs.h" // for N_TYPHOID_SEROTYPES
#include "SimulationEnums.h" // for PyDemoVirusTypes
#include "TransmissionGroupMembership.h"
#include <map>

namespace Kernel {

class ReportPyDemo : public Report
{
    GET_SCHEMA_STATIC_WRAPPER(ReportPyDemo)
public:
    ReportPyDemo();
    virtual ~ReportPyDemo() {};

    static IReport* ReportPyDemo::CreateReport() { return new ReportPyDemo(); }

    virtual bool Configure( const Configuration * inputJson );
    virtual void EndTimestep( float currentTime, float dt );

    virtual void LogIndividualData( Kernel::IndividualHuman * individual);
    virtual void LogNodeData( Kernel::INodeContext * pNC );

protected:
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map );
    virtual void postProcessAccumulatedData();

private:

    //TransmissionGroupMembership_t memberships;

#if USE_BOOST_SERIALIZATION
    friend class ::boost::serialization::access;
    template<class Archive>
    friend void serialize(Archive &ar, ReportPyDemo& report, const unsigned int v);
#endif
};

}
