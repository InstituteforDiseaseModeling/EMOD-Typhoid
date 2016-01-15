/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "IMigrationInfo.h"
#include "Migration.h"

static const char * _module = "IMigrationInfo";

namespace Kernel
{
    namespace MigrationFactory
    {
        IMigrationInfoFactory* ConstructMigrationInfoFactory( const ::Configuration *config, 
                                                              const std::string& idreference,
                                                              MigrationStructure::Enum ms,
                                                              bool useDefaultMigration,
                                                              int defaultTorusSize )
        {
            bool enable_migration = (ms != MigrationStructure::NO_MIGRATION);

            IMigrationInfoFactory* p_mif = nullptr ;
            if( useDefaultMigration )
            {
                p_mif = new MigrationInfoFactoryDefault( enable_migration, defaultTorusSize );
            }
            else
            {
                p_mif = new MigrationInfoFactoryFile( enable_migration );
            }
            p_mif->Initialize( config, idreference );
            return p_mif;
        }
    }
}