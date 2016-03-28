/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "stdafx.h"

#include "IdmApi.h"

#include <map>

#include "Sugar.h"

#define ENABLE_LOG_VALID 1  // clorton

namespace Logger
{
    typedef enum {
        CRITICAL,
        _ERROR, // ERROR breaks on msvc!
        WARNING,
        INFO,
        DEBUG,
        VALIDATION
    } tLevel;
};

// EVIL MACROS COMING UP! Idea here is that folks can log with 1 parameter (the string).
// Except for debug builds, LOG_VALID will compile to noop

#define LOG_LVL(lvl, x)          do { if((EnvPtr !=nullptr) && EnvPtr->Log->CheckLogLevel(Logger::lvl, _module))  EnvPtr->Log->Log(Logger::lvl, _module, x); } while(0)
#define LOG_LVL_F(lvl, x, ...)   do { if((EnvPtr !=nullptr) && EnvPtr->Log->CheckLogLevel(Logger::lvl, _module))  EnvPtr->Log->LogF(Logger::lvl, _module, x, ##__VA_ARGS__); } while(0)

#define LOG_LEVEL(lvl)          ((EnvPtr != nullptr) ? EnvPtr->Log->CheckLogLevel(Logger::lvl, _module) : false)

#define LOG_ERR(x)            LOG_LVL( _ERROR, x )
#define LOG_ERR_F(x, ...)     LOG_LVL_F( _ERROR, x, ##__VA_ARGS__ )
#define LOG_WARN(x)           LOG_LVL( WARNING, x )
#define LOG_WARN_F(x, ...)    LOG_LVL_F( WARNING, x, ##__VA_ARGS__ )
#define LOG_INFO(x)           LOG_LVL( INFO, x )
#define LOG_INFO_F(x, ...)    LOG_LVL_F( INFO, x, ##__VA_ARGS__ )
#define LOG_DEBUG(x)          LOG_LVL( DEBUG, x )
#define LOG_DEBUG_F(x, ...)   LOG_LVL_F( DEBUG, x, ##__VA_ARGS__ )
#if defined(_DEBUG) || defined(ENABLE_LOG_VALID)
#define LOG_VALID(x)          LOG_LVL( VALIDATION, x )
#define LOG_VALID_F(x, ...)   LOG_LVL_F( VALIDATION, x, ##__VA_ARGS__ )
#else
#define LOG_VALID(x)
#define LOG_VALID_F(x, ...)
#endif

namespace json
{
    class QuickInterpreter;
}

struct LogTimeInfo
{
    time_t hours;
    time_t mins;
    time_t secs;
};

struct cmp_str
{
   bool operator()(char const *a, char const *b) { return std::strcmp(a, b) < 0; }
};

class IDMAPI SimpleLogger
{
public:
    SimpleLogger();
    SimpleLogger( Logger::tLevel syslevel );
    void Init( const json::QuickInterpreter * configJson );
    bool CheckLogLevel( Logger::tLevel log_level, const char* module );
    void Log(Logger::tLevel log_level, const char* module, const char* msg);
    void LogF( Logger::tLevel log_level, const char* module, const char* msg, ...);
    void Flush();

    void GetLogInfo(LogTimeInfo &tInfo );

protected:

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
    typedef std::map< const char*, Logger::tLevel, cmp_str > module_loglevel_map_t;
    module_loglevel_map_t _logLevelMap;
#pragma warning( pop )

    Logger::tLevel _systemLogLevel;

    bool _throttle;
    bool _initialized;
    bool _flush_all;
    bool _warnings_are_fatal;

    time_t _initTime;
    int _rank;
};

struct LogLevel // for holding tags we will use in the log itself to indicate the level
{
    static const char * Valid;
    static const char * Debug;
    static const char * Info;
    static const char * Warning;
    static const char * Error;
};
