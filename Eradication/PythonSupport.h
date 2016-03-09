/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>

#ifdef ENABLE_PYTHON
#include "Python.h"
#endif

#include "Environment.h"

#define PythonSupportPtr static_cast<Kernel::PythonSupport*>(EnvPtr->pPythonSupport)

namespace Kernel
{
    class PythonSupport
    {
    public:
        static std::string SCRIPT_PRE_PROCESS;
        static std::string SCRIPT_POST_PROCESS;
        static std::string SCRIPT_POST_PROCESS_SCHEMA;
        static std::string SCRIPT_PYTHON_FEVER;

        PythonSupport();
        ~PythonSupport();

        void CheckPythonSupport( bool isGettingSchema, const std::string& pythonScriptPath );

#ifdef ENABLE_PYTHON
        PyObject* IdmPyInit( const char * python_script_name, const char * python_function_name );
#endif

        std::string RunPreProcessScript( const std::string& configFileName );
        void RunPostProcessScript( const std::string& outputPath );
        void RunPostProcessSchemaScript( const std::string& schemaPath );
        void CheckSimScripts( const std::string& simTypeStr );

    private:
        std::string CreatePythonScriptPath( const std::string& script_filename );
        bool PythonScriptCheckExists( const std::string&script_filename );
        void PythonScriptsNotFound();
        char* PythonHomePath();

        std::string m_PythonScriptPath;
        bool m_IsGettingSchema;
        bool m_CheckForSimScripts;
    };
}