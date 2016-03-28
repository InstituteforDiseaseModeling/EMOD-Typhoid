/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Sugar (Syntactic)

#pragma once

// Environment helpers
#include "Environment.h"
#define EnvPtr Environment::getInstance() // make code a bit more readable using Dll-boundary-crossable Env pointer

//////////////////////////////////////////////////////////
// helpers for config variables 

#define CONFIG_PARAMETER_EXISTS(config_ptr, name) \
 (config_ptr->As<json::Object>().Find(name) != config_ptr->As<json::Object>().End())
