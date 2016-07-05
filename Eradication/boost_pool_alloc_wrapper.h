/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

// taken from https://svn.boost.org/trac/boost/ticket/4346
// Think the right way to do this is to upgrade to boost 1.44

#pragma push_macro("malloc") 
#pragma push_macro("free") 
#undef malloc 
#undef free 

#include <boost/pool/pool_alloc.hpp>

#pragma pop_macro("malloc") 
#pragma pop_macro("free") 
