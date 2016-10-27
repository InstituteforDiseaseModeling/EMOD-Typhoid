/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#define STRINGIFY(ver)       #ver
#define XSTRINGIFY(ver)       STRINGIFY(ver)

#ifdef WIN32
static const char* BUILD_DATE = __DATE__ " " __TIME__;
static const char *SCCS_URL   = "https://github.com/InstituteforDiseaseModeling/DtkTrunk/commit/388e180";
#else
#define BUILD_DATE __DATE__
#endif

#ifndef SCCS_BRANCH
#define    SCCS_BRANCH      "Typhoid-Ongoing (388e180)"
#endif
#ifndef SCCS_DATE
#define    SCCS_DATE        "2016-08-22 14:06:07 -0700"
#endif

#define    MAJOR_VERSION    2
#define    MINOR_VERSION    5
#ifndef REVISION_NUMBER
#define    REVISION_NUMBER  1099
#endif
#define    BUILD_NUMBER     0


#define FULL_VERSION_WITH_SVN      XSTRINGIFY(MAJOR_VERSION) "." XSTRINGIFY(MINOR_VERSION) "." XSTRINGIFY(REVISION_NUMBER)
#define FULL_VERSION_WITH_SVN_NQ   MAJOR_VERSION,MINOR_VERSION,REVISION_NUMBER,BUILD_NUMBER
