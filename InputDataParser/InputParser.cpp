/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "parser.h"
#include <fstream>
#include <iostream>


int _tmain(int argc, _TCHAR* argv[])
{
    //std::cout<<"Working on Madagascar..."<<std::endl<<std::endl;
    //ReadInFilesForDemo("mdgp00ag.asc", "MadagascarSettlement.txt", "links_madagascar.txt", "ClimateType10082009.txt", "MadagascarAltitude.cgi", "MadDemographic.txt", "Mad_2_5arcminute_");
    //CreateUrbanExtent("mdgp00ag.asc", "mdgurextents.asc", "Mad_2_5arcminute_");
//    std::cout<<"Working on Zambia..."<<std::endl<<std::endl;
//    ReadInFilesForDemo("zmbp00ag.asc", "ZambiaSettlement.txt", "links_zambia.txt", "ClimateType10082009.txt", "ZambiaAltitude.cgi", "ZamDemographic.txt", "Zam_2_5arcminute_");
    std::cout<<"Working on India..."<<std::endl<<std::endl;
    ReadInFilesForDemo("indp10ag.asc", "settlements_india_reformatted.txt", "links_india.txt", "airports_india.txt", "airlinks_india_daily.txt", "ClimateType10082009.txt", "ind_topography.cgi", "demo_input_india.txt", "Ind_2_5arcminute_20100521_");
    //CreateUrbanExtent("zmbp00ag.asc", "zmburextents.asc", "Zam_2_5arcminute_");
    //std::cout<<"Working on India..."<<std::endl<<std::endl;
    //ReadInFilesForDemo("indp05ag.asc", "IndiaSettlement.txt", "links_india.txt", "ClimateType10082009.txt", "IndiaAltitude.cgi", "IndDemographic.txt", "Ind_2_5arcminute_");
    //std::cout<<"Working on Nigeria..."<<std::endl<<std::endl;
    //ReadInFilesForDemo("ngap00ag.asc", "NigeriaSettlement.txt", "links_nigeria.txt", "ClimateType10082009.txt", "NigeriaAltitude.cgi", "NigeriaDemographic.txt", "Nigeria_2_5arcminute_");

    return 0;
}
