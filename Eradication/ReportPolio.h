/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "ReportEnvironmental.h"
#include "PolioDefs.h" // for N_POLIO_SEROTYPES
#include "SimulationEnums.h" // for PolioVirusTypes
#include <map>

namespace Kernel {

class ReportPolio : public ReportEnvironmental
{
    GET_SCHEMA_STATIC_WRAPPER(ReportPolio)
public:
    ReportPolio();
    virtual ~ReportPolio() {};

    static IReport* ReportPolio::CreateReport() { return new ReportPolio(); }

    virtual bool Configure( const Configuration * inputJson );
    virtual void EndTimestep( float currentTime, float dt );

    virtual void LogIndividualData( Kernel::IIndividualHuman* individual);
    virtual void LogNodeData( Kernel::INodeContext * pNC );

protected:
    virtual void populateSummaryDataUnitsMap( std::map<std::string, std::string> &units_map );
    virtual void postProcessAccumulatedData();

    int cumulative_paralytic_cases[N_POLIO_VIRUS_TYPES];
    float total_cumulative_paralytic_cases;
    float cumulativeDiseaseSusceptibleInfections;
    typedef std::map< unsigned int, unsigned int > tStrainIdToOccurrencesMap;
    tStrainIdToOccurrencesMap infectionStrainIdCounts;
    float immunityAtInfection;
    float seroprevalence[N_POLIO_SEROTYPES];
    int total_infections;

    //specify the serotype which is used to report infected individuals who are susceptible to paralysis
    PolioVirusTypes::Enum DiseaseSusceptibleReportPoliovirusType;
};

}
