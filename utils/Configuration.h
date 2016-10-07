/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IdmApi.h"
#include <string>
#include <set>
#include <vector>

#include "BoostLibWrapper.h"
#include "ISupports.h"
#include "CajunIncludes.h"

#include "Serializer.h"

#include "JsonObject.h" // For IJsonSerializable

// simple wrapper around config information - reuse the QuickInterpreter functionality but 
// handle loading explicitly on creation
class Configuration : public json::QuickInterpreter
{
public:
    static Configuration* CopyFromElement(const json::Element &elem); // creates a Configuration object representing a copy of the subtree rooted at the element passed in

    bool CheckElementByName(const std::string& elementName) const;

    static Configuration* Load(std::string configFileName);
    static Configuration* Load(std::istream& rInputStream, const std::string& rDataLocation);
    static Configuration* LoadFromPython(const std::string &configFileName);
    virtual ~Configuration() { delete pElement; }

private:
    // Don't let users create an empty one.
    Configuration(json::Element* element) : QuickInterpreter(*element) { pElement = element; }

    static Configuration* loadInternal(std::istream &config_file);

    json::Element* pElement; // maintain the backing string containing the original data also

    Configuration() : QuickInterpreter(json::Element()), pElement(nullptr) {} // this ctor only for serialization, the base class will be initialized to an invalid state which deserialization must repair
};

class JsonUtility
{
public:
    static void logJsonException( const json::ParseException &pe, std::string& err_msg );
    static void logJsonException( const json::ScanException &pe, std::string& err_msg );
};

Configuration IDMAPI *Configuration_Load( const std::string& rFilename ) ;

/////////////////////////////////////////////////////////////////////////////////////////
// config/json loading wrappers

std::set< std::string > GET_CONFIG_STRING_SET(const json::QuickInterpreter* parameter_source, const char *name);
inline std::set< std::string > GET_CONFIG_STRING_SET(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_STRING_SET(parameter_source, name.c_str());
}

std::vector< std::string> GET_CONFIG_VECTOR_STRING(const json::QuickInterpreter* parameter_source, const char *name);
inline std::vector< std::string > GET_CONFIG_VECTOR_STRING(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_VECTOR_STRING(parameter_source, name.c_str());
}

std::vector< std::vector< std::string > > GET_CONFIG_VECTOR2D_STRING(const json::QuickInterpreter* parameter_source, const char *name);
inline std::vector< std::vector< std::string > > GET_CONFIG_VECTOR2D_STRING(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_VECTOR2D_STRING(parameter_source, name.c_str());
}

std::vector< float > GET_CONFIG_VECTOR_FLOAT(const json::QuickInterpreter* parameter_source, const char *name);
inline std::vector< float > GET_CONFIG_VECTOR_FLOAT(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_VECTOR_FLOAT(parameter_source, name.c_str());
}

std::vector< int > GET_CONFIG_VECTOR_INT(const json::QuickInterpreter* parameter_source, const char *name);
inline std::vector< int > GET_CONFIG_VECTOR_INT(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_VECTOR_INT(parameter_source, name.c_str());
}

std::vector< std::vector< float > > GET_CONFIG_VECTOR2D_FLOAT(const json::QuickInterpreter* parameter_source, const char *name);
inline std::vector< std::vector< float > > GET_CONFIG_VECTOR2D_FLOAT(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_VECTOR2D_FLOAT(parameter_source, name.c_str());
}

std::vector< std::vector< int > > GET_CONFIG_VECTOR2D_INT(const json::QuickInterpreter* parameter_source, const char *name);
inline std::vector< std::vector< int > > GET_CONFIG_VECTOR2D_INT(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_VECTOR2D_INT(parameter_source, name.c_str());
}

std::string GET_CONFIG_STRING(const json::QuickInterpreter* parameter_source, const char *name);
inline std::string GET_CONFIG_STRING(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_STRING(parameter_source, name.c_str());
}

#ifdef __GNUC__
double GET_CONFIG_DOUBLE(const json::QuickInterpreter* parameter_source, const char *name, int min = -1*std::numeric_limits<int>::max());
#else
double GET_CONFIG_DOUBLE(const json::QuickInterpreter* parameter_source, const char *name, int min = -1*INT_MAX);
#endif
inline double GET_CONFIG_DOUBLE(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_DOUBLE(parameter_source, name.c_str());
}

inline bool GET_CONFIG_BOOLEAN(const json::QuickInterpreter* parameter_source, const char *name)
{
    double d = GET_CONFIG_DOUBLE(parameter_source, name);

    if(d != 0.0 && d != 1.0)
        throw std::runtime_error(("Non-boolean value found for config parameter: " + std::string(name)).c_str());

    return (d == 1.0);
}

inline bool GET_CONFIG_BOOLEAN(const json::QuickInterpreter* parameter_source, const std::string& name)
{
    return GET_CONFIG_BOOLEAN(parameter_source, name.c_str());
}

#define GET_CONFIG_INTEGER(_cfg, _name)   (int)GET_CONFIG_DOUBLE(_cfg, _name)
#define GET_CONFIG_NUMBER(_cfg, _name)    (float)GET_CONFIG_DOUBLE(_cfg, _name)


//////////////////////////////////////////////////////////////////////////
// Declarations for Configurable object functionality

namespace Kernel
{
    using namespace std;
    using namespace json;

    ////////////////////////////////////////////////////////////////////////
    // Interface for classes to implement to serialize itself
    // to a JSon string via JsonObject library
    class JSerializer;
    struct IDMAPI IJsonSerializable : public ISupports
    {
        virtual void JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const = 0;
        virtual void JDeserialize( IJsonObjectAdapter* root, JSerializer* helper ) = 0;
    };

    //////////////////////////////////////////////////////////////////////
    // Configuration Mechanism

    struct IDMAPI IConfigurable : ISupports
    {
        virtual bool Configure(const Configuration *config) = 0;
        virtual QuickBuilder GetSchema() = 0;
    };

#define DECLARE_CONFIGURED(classname) \
protected: \
    template<class Mode> void common_configured_dispatch(const Configuration *config, /*caller supplies base object, out param*/ QuickBuilder *schema);\
public: \
    virtual bool Configure(const Configuration *config);
}
