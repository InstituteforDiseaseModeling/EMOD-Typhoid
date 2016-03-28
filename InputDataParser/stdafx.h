/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#include <stdio.h>
#include <tchar.h>
#include <string.h>

#define MAX_DIST 1000000 //in kilometers, the maximum possible distance in the code
#define MAX_NAME 20
#define PI 3.1415926535
//for regional migration
#define MAX_REGIONS 100
#define ROAD_RATE1 .05
#define ROAD_RATE2 .15
#define ROAD_RATE3 .5
#define RAIL_RATE4 .3
#define RAIL_RATE5 .55
#define COMM_REGION .01
#define ONEDAY_DISTANCE1 100
#define ONEDAY_DISTANCE2 300
#define ONEDAY_DISTANCE3 600
#define ONEDAY_DISTANCE4 200
#define ONEDAY_DISTANCE5 300
#define LAT_CONVERTER 111.18    //approximate conversion to km
#define LON_CONVERTER 85.29     //approximate conversion to km

//limits on maximum connections per city (road and air networks)
#define REGION_0_PADDING 30
#define AIR_0_PADDING 60

//for local migration
#define LOCAL_MIGRATE 100 // number of people moving to each adjacent square for larger pops, if less than 800 in a square, then the rate is 1/8 to leave to each adjacent square, 
// so that sparse areas are more migratory.  This number can be tuned

//for air migration
//1 -- for each community to airport, link is based on the proportion of population (flat travel rate)
//2 -- for each community to airport, link is based on both the proportion of population and the distance to the airport, rate is decayed with normal distribution.
#define AIRBASIN_LINK_DIST_FLAG 2 //This flag defines how people will travel to the airport
#define PERCENT_ANNUAL_SCALING 1 //reflects the relative change compared to 2010 (the year which data are used)
#define PERCENT_PASSENGER_FLIGHT 0.7 //the fraction for how many fraction of passengers occupying the flight
#define DIST_THRESHOLD 200 //maximum radius for a voronoi basin in kilometers
#define SIGMA 50 //sigma value for normal distribution of decay in comm-to-air migration