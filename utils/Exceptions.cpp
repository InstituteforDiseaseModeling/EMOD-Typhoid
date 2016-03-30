/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <stdafx.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <boost/lexical_cast.hpp> // no!!!
#include "Exceptions.h"
#include "Types.h" // temp, for backtrace
#ifdef __GNUC__
#include <execinfo.h>
#else

#include <windows.h>
#include <Dbghelp.h> // requires Dbghelp.lib be on included as a library
#include <winnt.h>
#include <WinBase.h>

#endif

static inline std::string dump_backtrace()
{
    std::stringstream stack_trace ;
    const int MAX_STACK = 100 ;
    void * stack[MAX_STACK];

#ifdef __GNUC__
    // get void*'s for all entries on the stack
    int nptrs = backtrace( stack, MAX_STACK );

    // print out all the frames to stderr
    // Note this is still mangled. Run output through c++filt and you're done.
    //backtrace_symbols_fd( stack, nptrs, STDERR_FILENO );

    char** strings = backtrace_symbols( stack, nptrs );
    if( strings != nullptr )
    {
        for( int j = 0; j < nptrs ; j++ )
        {
            stack_trace << std::string( strings[j] ) << std::endl ;
        }

        free(strings);
    }
#else

    HANDLE process = GetCurrentProcess();

    // ------------------------------
    // --- Initialize symbol handler
    // ------------------------------
    // need to give a path if we put the PDB file some place 
    // other than the current working directory.
    PCSTR user_search_path = nullptr ;

    if( !SymInitialize( process, user_search_path, TRUE ) )
    {
        DWORD error = GetLastError();
        return stack_trace.str() ;
    }

    // -----------------------
    // --- Get the stack trace
    // -----------------------
    // NOTE: _WIN32_WINNT needs to be set in the stdafx.h file to at least _WIN32_WINNT_VISTA (0x0600)
    //       so that RtlCaptureStackBackTrace() is defined.
    // The first parameters is 3 because the user doesn't need the following lines:
    // 1 - dump_backtrace()
    // 2 - DetailedException constructor
    // 3 - Derived Exception constructor
    ULONG frames_to_skip = 3 ;
    ULONG frames_to_capature = MAX_STACK ;
    unsigned short num_frames = CaptureStackBackTrace( frames_to_skip, frames_to_capature, stack, nullptr );

    // -------------------------------------------------
    // --- Prepare memory for getting symbol information
    // -------------------------------------------------
    SYMBOL_INFO*   symbol = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 1024 * sizeof( char ), 1 );
    symbol->MaxNameLen   = 1023;
    symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

    // ------------------------------------------------------
    // --- Set the options for getting symbol information.
    // --- Needed for SymGetLineFromAddr64().
    // --- SYMOPT_LOAD_LINES = Load line number information.
    // ------------------------------------------------------
    SymSetOptions( SYMOPT_LOAD_LINES );

    DWORD  dwDisplacement = 0;
    IMAGEHLP_LINE64 line;
    line.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

    // ----------------------------------------------------------------
    // --- Add a line to the string stream for each frame in the stack
    // ----------------------------------------------------------------
    for( int i = 0; i < num_frames; i++ )
    {
        std::string method_name = "???" ;
        std::string address     = "???" ;
        std::string file_name   = "???" ;
        int line_number = 0 ;

        // -------------------------------------
        // --- Gets the method name in the stack
        // -------------------------------------
        if( SymFromAddr( process, DWORD64(stack[ i ]), nullptr, symbol ) )
        {
            method_name = symbol->Name ;
            char buff[50] ;
            sprintf_s(buff,"0x%0X", symbol->Address );
            address = std::string(buff);
        }

        // ----------------------------------------------
        // --- Gets the file and line number information
        // ----------------------------------------------
        if( SymGetLineFromAddr64( process, DWORD64(stack[ i ]), &dwDisplacement, &line ) )
        {
            file_name   = line.FileName ;
            line_number = line.LineNumber ;
        }
        else
        {
            file_name = address ;
        }

        // --------------------------------------------------------------------------
        // --- Doing "filepath(linenumber)" works in Visual stuido output window
        // --- such that double clicking on it takes you to that file and line number.
        // --------------------------------------------------------------------------
        stack_trace << file_name << "(" << line_number << "): " << method_name << "\n"  ;
    }

    free( symbol );
#endif
    return stack_trace.str() ;
}

// This is an exception library for the DTK. We wanted to keep it relatively flat and 
// use-case driven, not theoretical. We have a DetailedException class that we use as our base that
// let's you pass information from the site of the exception. We want to move away from user-facing
// error message strings being passed in by the caller.
//
// Here's the Exceptions TOC with some clues about their usage:
// * BadEnumInSwitchStatementException [switch...default]
// * BadMapKeyException [ some_map.find( key ) == some_map.end() ]
// * CalculatedValueOutOfRangeException
// * ConfigurationRangeException (should become used only in JsonConfigurable as heterogeneous intra-node transmission is moved to initConfig)
// * DllLoadingException
// * FactoryCreateFromJsonException
// * FileIOException
// * FileNotFoundException
// * GeneralConfigurationException (try to use a more specific Configuration exception)
// * IllegalOperationException (try to use a more specific exception)
// * IncoherentConfigurationException
// * InitializationException (try to use a more specific exception; why did the initialization fail?)
// * InvalidInputDataException (sort of like a parsing exception, may create specific XXXParserExceptions)
// * MPIException (limited lifespan)
// * NotYetImplementedException (need to use more of these)
// * NullPointerException (really try to avoid using this; why is a pointer null?)
// * OutOfRangeException (consider whether should be ConfigurationRangeEx or CalculatedValueRangeEx; do we need an ArrayIndexOutOfBoundsEx?)
// * QueryInterfaceException
// * SerializationException
//

static const char* default_varname = "variable name";
static const char* default_description = "description";
#define GET_VAR_NAME(v) ( (v) ? (v) : default_varname )
#define GET_DES_STR(s)  ( (s) ? (s) : default_description )

namespace Kernel {
#if 0
    DetailedException::DetailedException( const char * msg, const char * file_name, int line_num )
    : std::runtime_error( msg )
    , _fileName( file_name )
    , _lineNum( line_num )
    , _msg( msg )
    {
        //std::cout << "DetailedException ctor: msg = " << msg << ", file = " << file_name << ", line_num = " << line_num << std::endl;
    }
#endif

    DetailedException::DetailedException( const char * file_name, int line_num, const char * func_name )
    : std::runtime_error( std::string( "\nException in " ) + file_name + " at " + boost::lexical_cast<std::string>(line_num) + " in " + func_name + ".\n" )
    , _msg()
    , _stackTrace()
    , _fileName( file_name )
    , _funcName( func_name )
    , _lineNum( line_num )
    {
        //std::cout << "DetailedException ctor: msg = " << msg << ", file = " << file_name << ", line_num = " << line_num << std::endl;
        _stackTrace = dump_backtrace();
    }

    //DetailedException::~DetailedException() throw() {}; // or some compilers complain

    // this used to return what(), but now it returns our customized string.
    // what() can always be called directly in case our carefully formed message
    // from parameters lost something really useful! Callers should not assume
    // _msg includes what().
    const char *
    DetailedException::GetMsg() const
    {
        return _msg.c_str();
    }
    const char *
    DetailedException::GetFilename() const { return _fileName; }
    int
    DetailedException::GetLineNumber() const { return _lineNum; }
    const char*
    DetailedException::GetFunction() const { return _funcName; }

    const std::string&
    DetailedException::GetStackTrace() const 
    {
        return _stackTrace ;
    }

    // This is to be used in the default section of most switch statements. 
    BadEnumInSwitchStatementException::BadEnumInSwitchStatementException( const char* file_name, int line_num, const char* function_name, const char* var_name, int bad_value, const char* as_string )
    : DetailedException( file_name, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "BadEnumInSwitchStatementException: "
                 << what()
                 << "Value "
                 << bad_value 
                 << "("
                 << GET_DES_STR(as_string)
                 << ")"
                 << " of variable "
                 << GET_VAR_NAME(var_name)
                 << " not handled by switch statement.";
        _msg = _tmp_msg.str();
    }

    // This is to be used when a map is searched for a key that is believed
    // to exist but is not found.
    BadMapKeyException::BadMapKeyException( const char* file_name, int line_num, const char* function_name, const char* var_name, const char* value )
    : DetailedException( file_name, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "BadMapKeyException: "
                 << what() 
                 << "Failed to find "
                 << value
                 << " in map "
                 << GET_VAR_NAME(var_name);
        _msg = _tmp_msg.str();
    }

    CalculatedValueOutOfRangeException::CalculatedValueOutOfRangeException( const char* file_name, int line_num, const char* function_name, const char* var_name, float bad_value, float range_violated )
    : DetailedException( file_name, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "CalculatedValueOutOfRangeException: "
                 << what()
                 << GET_VAR_NAME( var_name )
                 << " has value "
                 << bad_value
                 << " that violates range constraint "
                 << range_violated;
        _msg = _tmp_msg.str();
    }

    ConfigurationRangeException::ConfigurationRangeException( const char * file_name, int line_num, const char * func_name, const char* var_name, float var_value, float test_value )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "ConfigurationRangeException: "
                 << what()
                 << "Configuration variable " 
                 << GET_VAR_NAME(var_name)
                 << " with value " 
                 << var_value 
                 << " out of range: " << ( (var_value<=test_value) ? "less than or equal to " : "greater than " )
                 << test_value
                 << ".";
        _msg = _tmp_msg.str();
    }
    
    DllLoadingException::DllLoadingException( const char * file_name, int line_num, const char * func_name, const char * msg )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "DllLoadingException: filename = " << what() << msg << std::endl;
        _msg = _tmp_msg.str();
    }

    FactoryCreateFromJsonException::FactoryCreateFromJsonException( const char* file_name, int line_num, const char* func_name, const char* note )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "FactoryCreateFromJsonException: " << what() << note << std::endl;
        _msg = _tmp_msg.str();
    }

    FileIOException::FileIOException( const char* sourccode_filename, int line_num, const char* function_name, const char* msg )
    : DetailedException( sourccode_filename, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "FileIOException: "
                 << what() 
                 << "I/O error while reading/writing file. Hint: "
                 << msg;
        _msg = _tmp_msg.str();
    }

    FileNotFoundException::FileNotFoundException( const char * src_file_name, int line_num, const char* func_name, const char * missing_file_name )
    : DetailedException( src_file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "FileNotFoundException: "
            << what()
            << "Could not find file "
            << missing_file_name;
        _msg = _tmp_msg.str();
    }

    GeneralConfigurationException::GeneralConfigurationException( const char * file_name, int line_num, const char* func_name, const char * msg )
    : DetailedException( file_name, line_num, func_name )
    {
        // pass message straight through
        std::ostringstream _tmp_msg;
        _tmp_msg << "GeneralConfigurationException: " << what() << msg << std::endl;
        _msg = _tmp_msg.str();
    }

    IllegalOperationException::IllegalOperationException( const char * file_name, int line_num, const char* func_name, const char * msg )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "IllegalOperationException: " << what() << msg << std::endl;
        _msg = _tmp_msg.str();
    }

    IncoherentConfigurationException::IncoherentConfigurationException( const char * file_name, int line_num, const char* func_name, const char* existing_label, float existing_value, const char* test_label, float test_value, const char* details )
    : DetailedException( file_name, line_num, func_name )
    {
        createICEMessage( existing_label, boost::lexical_cast<std::string>(existing_value).c_str(), test_label, boost::lexical_cast<std::string>(test_value).c_str(), details );
    }

    IncoherentConfigurationException::IncoherentConfigurationException(
        const char * file_name,
        int line_num,
        const char* func_name,
        const char* existing_label,
        const char* existing_value,
        const char* test_label,
        const char* test_value,
        const char* details
    )
    : DetailedException( file_name, line_num, func_name )
    {
        createICEMessage( existing_label, existing_value, test_label, test_value, details );
    }

    void IncoherentConfigurationException::createICEMessage(
        const char* existing_label,
        const char* existing_value,
        const char* test_label,
        const char* test_value,
        const char* details
    )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "IncoherentConfigurationException: "
            << what()
            << "Variable or parameter '"
            << existing_label
            << "' with value "
            << existing_value
            << " is incompatible with variable or parameter '"
            << test_label
            << "' with value "
            << test_value
            << ". "
            << details;
        _msg = _tmp_msg.str();
    }

    InitializationException::InitializationException( const char * file_name, int line_num, const char * func_name, const char * msg )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "InitializationException: " << what() << msg;
        _msg = _tmp_msg.str();
    }

    InvalidInputDataException::InvalidInputDataException( const char* file_name, int line_num, const char* func_name, const char* note )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "InvalidInputDataException: " << what() << note << std::endl;
        _msg = _tmp_msg.str();
    }

    NodeDemographicsFormatErrorException::NodeDemographicsFormatErrorException( const char* file_name, 
                                                                                int line_num, 
                                                                                const char* func_name, 
                                                                                const char* demographicsFilename, 
                                                                                const char* note )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "NodeDemographicsFormatErrorException: " << what() ;
        _tmp_msg << "Format error encountered loading demographics file (" << demographicsFilename << ").  " ;
        _tmp_msg << note << std::endl;
        _msg = _tmp_msg.str();
    }

    MPIException::MPIException( const char* file_name, int line_num, const char* func_name, const char* note )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "MPIException: " << what() << note << std::endl;
        _msg = _tmp_msg.str();
    }

    NotYetImplementedException::NotYetImplementedException( const char * file_name, int line_num, const char* func_name, const char * msg )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "NotYetImplementedException: "
                 << what()
                 << msg
                 << std::endl;
        _msg = _tmp_msg.str();
    }

    NullPointerException::NullPointerException( const char * file_name, int line_num, const char * func_name, const char* var_name, const char* type_name )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "NullPointerException: "
                 << what()
                 << "Variable "
                 << GET_VAR_NAME(var_name)
                 << " was NULL.";
        _msg = _tmp_msg.str();
    }
    
    OutOfRangeException::OutOfRangeException( const char * file_name, int line_num, const char* func_name, const char* var_name, float value, float value_violated )
    : DetailedException( file_name, line_num, func_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "OutOfRangeException: "
                 << what()
                 << "Variable "
                 << GET_VAR_NAME(var_name)
                 << " had value " 
                 << std::setprecision(9) << value 
                 << " which was inconsistent with range limit "
                 << std::setprecision(9) << value_violated;
        _msg = _tmp_msg.str();
    }

    QueryInterfaceException::QueryInterfaceException( const char* file_name, int line_num, const char* function_name, const char* var_name, const char* queried_for_type, const char* variable_type )
    : DetailedException( file_name, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "QueryInterfaceException: "
                 << what()
                 << "QueryInterface on variable "
                 << GET_VAR_NAME(var_name)
                 << " of type "
                 << variable_type
                 << " failed to find interface "
                 << queried_for_type
                 << ".";
        _msg = _tmp_msg.str();
    }

    SerializationException::SerializationException( const char* filename, int line_num, const char* function_name, const char* notes )
    : DetailedException( filename, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "SerializationException: "
                 << what()
                 << notes
                 << std::endl;
        _msg = _tmp_msg.str();
    }

    WarningException::WarningException( const char* filename, int line_num, const char* function_name )
    : DetailedException( filename, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "WarningException: "
                 << what()
                 << std::endl;
        _msg = _tmp_msg.str();
    }

    MissingParameterFromConfigurationException::MissingParameterFromConfigurationException(
        const char* filename,
        int line_num,
        const char* function_name,
        const char* config_file_name,
        const char* param_name
    )
    : DetailedException( filename, line_num, function_name )
    {
        std::ostringstream _tmp_msg;
        _tmp_msg << "MissingParameterFromConfigurationException: "
                 << what()
                 << "Parameter '"
                 << param_name
                 << "' not found in input file '"
                 << config_file_name
                 << "'."
                 << std::endl;
        _msg = _tmp_msg.str();
    }

    JsonTypeConfigurationException::JsonTypeConfigurationException(
        const char* filename,
        int line_num,
        const char* function_name,
        const char * param_name,
        const json::QuickInterpreter& json_blob, 
        const char * caught_msg )
    : DetailedException( filename, line_num, function_name )
    {
#if 0
                 << what()
                 << "Parameter '"
                 << param_name
                 << "' not found in input file '"
                 << config_file_name
                 << "'."
                 << std::endl;
#endif
        std::stringstream blob_msg;
        json::Writer::Write( json_blob, blob_msg );

        std::ostringstream _tmp_msg;
        _tmp_msg << "JsonTypeConfigurationException: "
                 << what()
                 << "While trying to parse json data for param >>> "
                 << param_name
                 << " <<< in otherwise valid json segment... "
                 << std::endl
                 << blob_msg.str()
                 << std::endl
                 << "Caught exception msg below: "
                 << std::endl
                 << caught_msg;
        _msg = _tmp_msg.str();
    }
}
