/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "SocietyFactory.h"
#include "SocietyImpl.h"

namespace Kernel {

    ISociety* SocietyFactory::CreateSociety( IRelationshipManager* manager )
    {
        ISociety* pSociety = SocietyImpl::Create( manager );
        return pSociety;
    }
}