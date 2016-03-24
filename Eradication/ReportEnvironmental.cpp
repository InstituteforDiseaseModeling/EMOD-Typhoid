/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "ReportEnvironmental.h" // for base class

ReportEnvironmental::ReportEnvironmental()
{}

void
ReportEnvironmental::populateSummaryDataUnitsMap(
    std::map<std::string, std::string> &units_map
)
{
    Report::populateSummaryDataUnitsMap(units_map);

    // Additional vector channels
    //units_map["Adult Environmentals"]         = "Environmentals";
}

#endif // ENABLE_POLIO
