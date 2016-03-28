/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <iostream>

namespace TestUtils
{
    template<class OArchiveT, class ThingT>
    bool save_thing(/*const breaks if I cant make serialize() const?*/ ThingT *thing, const char * filename)
    {
        // make an archive
        std::ofstream ofs(filename, std::ios::binary);
        if (!ofs.is_open()) 
        {
            log_error_printf("save_thing() failed to open file %s for writing.\n", filename);
            return false;
        }

        OArchiveT oa(ofs);
        oa << (*thing);
        return true;
    }

    template<class IArchiveT, class ThingT>
    ThingT* load_thing(const char * filename, ThingT *empty_thing)
    {
        // open the archive
        std::ifstream ifs(filename, std::ios::binary);
        if (!ifs.is_open())
        {
            log_error_printf("load_thing() failed to open file %s for reading.\n", filename);
            return NULL;
        }

        IArchiveT ia(ifs);

        ia >> (*empty_thing);

        return empty_thing;    
    }
}
