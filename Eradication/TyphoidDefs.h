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

#define N_TYPHOID_SEROTYPES   (3)

#define N_TYPHOID_VIRUS_TYPES (6)

#define N_TYPHOID_VACCINES    (6)


#define IS_WILD_TYPE( x ) ( x >= TyphoidVirusTypes::WPV1 && x <= TyphoidVirusTypes::WPV3 )
#define IS_SABIN_TYPE( x ) ( x >= TyphoidVirusTypes::VRPV1 && x <= TyphoidVirusTypes::VRPV3 )
#define MAX_TYPHOID_VACCINE_DOSES 12
#define MAX_VACC_TARGET_AGE 5
#define MAX_VACC_TARGET_AGE_IN_DAYS (MAX_VACC_TARGET_AGE*DAYSPERYEAR)

