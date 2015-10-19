/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#define N_POLIO_SEROTYPES   (3)

#define N_POLIO_VIRUS_TYPES (6)

#define N_POLIO_VACCINES    (6)


#define IS_WILD_TYPE( x ) ( x >= PolioVirusTypes::WPV1 && x <= PolioVirusTypes::WPV3 )
#define IS_SABIN_TYPE( x ) ( x >= PolioVirusTypes::VRPV1 && x <= PolioVirusTypes::VRPV3 )
#define MAX_POLIO_VACCINE_DOSES 12
#define MAX_VACC_TARGET_AGE 5
#define MAX_VACC_TARGET_AGE_IN_DAYS (MAX_VACC_TARGET_AGE*DAYSPERYEAR)

