/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "FileSystem.h"
#include "BaseTextReport.h"
#include "Sugar.h"
#include "IdmMpi.h"

static const char* _module = "BaseTextReport";

namespace Kernel {

    BaseTextReport::BaseTextReport( const std::string& rReportName, bool everyTimeStep )
        : BaseReport()
        , write_every_time_step( everyTimeStep )
        , report_name( rReportName )
        , output_stream()
        , reduced_stream()
        , outfile()
    {
    }

    BaseTextReport::~BaseTextReport()
    {
    }

    void BaseTextReport::Initialize( unsigned int nrmSize )
    {
        if( EnvPtr->MPI.Rank == 0)
        {
            std::string file_path = GetOutputFilePath();
            if( FileSystem::FileExists( file_path ) )
            {
                FileSystem::RemoveFile( file_path );
            }

            outfile.open( file_path.c_str() , std::ios_base::app );
            if( !outfile.is_open() )
            {
                throw Kernel::FileIOException( __FILE__, __LINE__, __FUNCTION__, file_path.c_str() );
            }

            WriteData( GetHeader() + "\n" );
        }
    }

    void BaseTextReport::EndTimestep(float currentTime, float dt)
    {
        if( write_every_time_step )
        {
            GetDataFromOtherCores();
            if( EnvPtr->MPI.Rank == 0 )
            {
                WriteData( reduced_stream.str() );
                reduced_stream.str( std::string() ); // clear stream
            }
        }
    }

    void BaseTextReport::Reduce()
    {
        if( !write_every_time_step )
        {
            GetDataFromOtherCores();
        }
    }

    void BaseTextReport::Finalize()
    {
        if( EnvPtr->MPI.Rank == 0 )
        {
            if( !write_every_time_step )
            {
                WriteData( reduced_stream.str() );
                reduced_stream.str( std::string() ); // clear stream
            }

            outfile.close();
        }
    }

    std::string BaseTextReport::GetOutputFilePath() const
    {
        std::string file_path = FileSystem::Concat( std::string(EnvPtr->OutputPath), GetReportName() );
        return file_path ;
    }
    
    void BaseTextReport::GetDataFromOtherCores()
    {
        std::string to_send = output_stream.str();
        std::string received;

        EnvPtr->MPI.p_idm_mpi->GatherToRoot( to_send, received );
        if (EnvPtr->MPI.Rank == 0)
        {
            reduced_stream << received;
        }

        output_stream.str(std::string());   // Clear the output stream.
    }

    void BaseTextReport::WriteData( const std::string& rStringData )
    {
        if( rStringData.empty() )
        {
            return ;
        }

        outfile << rStringData ;

        // -------------------------------------------
        // --- Flush the buffer so we don't lose data
        // -------------------------------------------
        // !!! The following line is commented out to save time.
        // !!! My one data point was running one scenario one time:
        // !!! 16:28 vs 17:17.  Not a huge difference but some.
        // !!! outfile.flush();
    }
}

