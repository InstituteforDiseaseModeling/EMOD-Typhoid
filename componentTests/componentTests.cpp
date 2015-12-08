/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "UnitTest++.h"
#include "IdmInitialize.h"
#include <mpi.h>

using namespace std;

int main(int argc, char* argv[])
{
    // ----------------------------------------------------------------------------------
    // --- IdmInitialize() - This is the method that connects the unit tests
    // --- to Eradication.exe.  It is called once at the beginning.
    // --- I have not verified this, but this should mean that all static code/variables
    // --- are created/run once for the unit tests.
    // ----------------------------------------------------------------------------------
    IdmInitialize();
    MPI_Init(nullptr, nullptr);

    if( argc == 2 )
    {
        return UnitTest::RunSuite( argv[1] );
    }
    else
    {
        return UnitTest::RunAllTests();
    }
}

void PrintDebug( const std::string& rMessage )
{
#ifdef WIN32
    std::wostringstream msg ;
    msg << rMessage.c_str() ;
    OutputDebugStringW( msg.str().c_str() );
#endif
}
