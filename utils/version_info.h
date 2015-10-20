/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#define STRINGIFY(ver)       #ver
#define XSTRINGIFY(ver)       STRINGIFY(ver)

#ifdef WIN32
static const char *BUILD_DATE   = "2015/08/07 14:40:43";
static const char *SVN_URL      = "https://idm-repo/svn/Eradication/trunk";
#else
static const char *SVN_URL      = XSTRINGIFY(SVN_BRANCH_FROM_SCONS);
#define BUILD_DATE __DATE__
#endif

#define    MAJOR_VERSION    2
#define    MINOR_VERSION    0
#ifndef REVISION_NUMBER
#define    REVISION_NUMBER  5538
#endif
#define    BUILD_NUMBER     0


#define FULL_VERSION_WITH_SVN      XSTRINGIFY(MAJOR_VERSION) "." XSTRINGIFY(MINOR_VERSION) "." XSTRINGIFY(REVISION_NUMBER)
#define FULL_VERSION_WITH_SVN_NQ   MAJOR_VERSION,MINOR_VERSION,REVISION_NUMBER,BUILD_NUMBER
