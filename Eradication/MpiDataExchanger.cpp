/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "MpiDataExchanger.h"
#include "BinaryArchiveWriter.h"
#include "BinaryArchiveReader.h"
#include "Environment.h"
#include "IdmDateTime.h"
#include "Sugar.h"
#include "Log.h"
#include "mpi.h"

static const char * _module = "MpiDataExchanger";

namespace Kernel
{
    MpiDataExchanger::MpiDataExchanger( const char* pName,
                                        WithSelfFunc withSelfFunc,
                                        SendToOthersFunc toOthersFunc,
                                        ReceiveFromOthersFunc fromOthersFunc,
                                        ClearDataFunc clearFunc )
        : m_pName( pName )
        , m_WithSelfFunc( withSelfFunc )
        , m_ToOthersFunc( toOthersFunc )
        , m_FromOthersFunc( fromOthersFunc )
        , m_ClearDataFunc( clearFunc )
    {
    }

    MpiDataExchanger::~MpiDataExchanger()
    {
    }

    void MpiDataExchanger::ExchangeData( IdmDateTime& currentTime )
    {
        std::vector< uint32_t > message_size_by_rank( EnvPtr->MPI.NumTasks );   // "buffers" for size of buffer messages
        std::vector< MPI_Request > outbound_requests;     // requests for each outbound message
        std::vector< BinaryArchiveWriter* > outbound_messages;  // buffers for outbound messages

        for (int destination_rank = 0; destination_rank < EnvPtr->MPI.NumTasks; ++destination_rank)
        {
            if (destination_rank == EnvPtr->MPI.Rank)
            {
                m_WithSelfFunc( destination_rank );
            }
            else
            {
                auto binary_writer = new BinaryArchiveWriter();
                IArchive* writer = static_cast<IArchive*>(binary_writer);

                m_ToOthersFunc( writer, destination_rank );

                if( EnvPtr->Log->CheckLogLevel(Logger::VALIDATION, _module) )
                {
                    WriteJson( int(currentTime.time), EnvPtr->MPI.Rank, destination_rank, "send", writer->GetBuffer(), writer->GetBufferSize() );
                }

                uint32_t buffer_size = message_size_by_rank[destination_rank] = writer->GetBufferSize();
                MPI_Request size_request;
                MPI_Isend( &message_size_by_rank[destination_rank], 1, MPI_UNSIGNED, destination_rank, 0, MPI_COMM_WORLD, &size_request );

                if (buffer_size > 0)
                {
                    const char* buffer = writer->GetBuffer();
                    MPI_Request buffer_request;
                    MPI_Isend( const_cast<char*>(buffer), buffer_size, MPI_BYTE, destination_rank, 0, MPI_COMM_WORLD, &buffer_request );
                    outbound_requests.push_back( buffer_request );
                    outbound_messages.push_back( binary_writer );
                }
            }

            m_ClearDataFunc( destination_rank );
        }

        for (int source_rank = 0; source_rank < EnvPtr->MPI.NumTasks; ++source_rank)
        {
            if (source_rank == EnvPtr->MPI.Rank) continue;  // We don't use MPI to send individuals to ourselves.

            uint32_t size;
            MPI_Status status;
            MPI_Recv(&size, 1, MPI_UNSIGNED, source_rank, 0, MPI_COMM_WORLD, &status);

            if (size > 0)
            {
                unique_ptr<char[]> buffer(new char[size]);
                MPI_Status buffer_status;
                MPI_Recv(buffer.get(), size, MPI_BYTE, source_rank, 0, MPI_COMM_WORLD, &buffer_status);

                if( EnvPtr->Log->CheckLogLevel(Logger::VALIDATION, _module) ) 
                {
                    WriteJson( int(currentTime.time), source_rank, EnvPtr->MPI.Rank, "recv", buffer.get(), size );
                }

                auto binary_reader = make_shared<BinaryArchiveReader>(buffer.get(), size);
                IArchive* reader = static_cast<IArchive*>(binary_reader.get());

                if( reader->HasError() )
                {
                    WriteJson( int(currentTime.time), source_rank, EnvPtr->MPI.Rank, "recv", buffer.get(), size );
                }

                m_FromOthersFunc( reader, source_rank );

                m_ClearDataFunc( source_rank );
            }
        }

        // Clean up from Isend(s)
        std::vector<MPI_Status> status( outbound_requests.size() );
        MPI_Waitall( outbound_requests.size(), (MPI_Request*)outbound_requests.data(), (MPI_Status*)status.data() );

        for (auto writer : outbound_messages)
        {
            delete writer;
        }
    }

    void MpiDataExchanger::WriteJson( uint32_t time_step,
                                      uint32_t source, 
                                      uint32_t dest, 
                                      char* suffix, 
                                      const char* buffer, 
                                      size_t size )
    {
        char filename[256];
    #ifdef WIN32
        sprintf_s(filename, 256, "%s\\%03d-%02d-%02d-%s-%s.json", EnvPtr->OutputPath.c_str(), time_step, source, dest, m_pName, suffix);
        FILE* f = nullptr;
        errno = 0;
        if ( fopen_s( &f, filename, "w" ) != 0)
        {
            // LOG_ERR_F( "Couldn't open '%s' for writing (%d - %s).\n", filename, errno, strerror(errno) );
            LOG_ERR_F( "Couldn't open '%s' for writing (%d).\n", filename, errno );
            return;
        }
    #else
        sprintf(filename, "%s\\%03d-%02d-%02d-%s-%s.json", EnvPtr->OutputPath.c_str(), time_step, source, dest, m_pName, suffix);
        FILE* f = fopen(filename, "w");
    #endif
        fwrite(buffer, 1, size, f);
        fflush(f);
        fclose(f);
    }
}