/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "IdmApi.h"
#include <string>
#include <functional>
#include "ISupports.h"  
#include "Configuration.h"
#include "Sugar.h"
#include "Environment.h"
#include <typeinfo>
#include <stdio.h>
#include "Exceptions.h"

#if defined(WIN32)
#define DTK_DLLEXPORT   __declspec(dllexport)
#else // Other non-windows platform
#define DTK_DLLEXPORT   
#endif

namespace Kernel
{
    using namespace std;
    using namespace json;

    //////////////////////////////////////////////////////////////////////////
    // CreateInstance/ClassFactory helpers
   
    typedef std::function<ISupports* (void)> instantiator_function_t;
    typedef map<string, instantiator_function_t> support_spec_map_t;

    static const char hexval[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    template <class ReturnTypeT>
    ReturnTypeT* CreateInstanceFromSpecs(const Configuration* config, support_spec_map_t &specs, bool query_for_return_interface = true)
    {
        string classname = GET_CONFIG_STRING(config, "class");
        ISupports* obj = nullptr;

        map<string, instantiator_function_t>::iterator it = specs.find(classname);
        if (specs.end() == it)
        {
            std::ostringstream errMsg;
            errMsg << "Could not instantiate unknown class '"
                   << classname
                   << "'."
                   << std::endl;
            throw FactoryCreateFromJsonException( __FILE__, __LINE__, __FUNCTION__, errMsg.str().c_str() );
        } 
        else
        {
            obj = it->second(); // create object
            obj->AddRef(); // increment reference counting for 'obj'

            IConfigurable *conf_obj = nullptr;
            if (s_OK == obj->QueryInterface(GET_IID(IConfigurable), (void**)&conf_obj))
            {
                if (!conf_obj->Configure(config))
                {
                    // release references to the objects
                    conf_obj->Release();
                    obj->Release();
                    return nullptr;
                }
            }
            else
            {
                // should we throw an exception?
            }
            if( conf_obj ) conf_obj->Release();  // release reference as 'conf_obj' is going out of scope.
        }

        /* now return an interface type the user actually wants*/
        if (query_for_return_interface)
        {
            ReturnTypeT *ri = nullptr;

            // get iid. Interesting issue here where macros and templates args interact unpredictably.
            string templateClassName = typeid(ReturnTypeT).name();
            templateClassName = templateClassName.substr( templateClassName.find_last_of("::")+1 );

            // debug logging
            //char iidStr[17];
            //_snprintf( iidStr, 17, "%x", (char*)(TypeInfo<ReturnTypeT>::GetIID((char*)templateClassName.c_str()).data) );
#ifdef _IID_DEBUG
            const char * hval = (const char*)TypeInfo<ReturnTypeT>::GetIID((char*)templateClassName.c_str()).data;
            char finalhash[32];
            memset( finalhash, '\0', 32 );

            for(int j = 0; j < 10; j++)
            {
                finalhash[j*2] = hexval[((hval[j] >> 4) & 0xF)];
                finalhash[(j*2) + 1] = hexval[(hval[j]) & 0x0F];
            }

            std::cerr << "[DEBUG] classname = " << typeid(ReturnTypeT).name() << ", iid = " << finalhash << std::endl;
#endif 
            if( s_OK != obj->QueryInterface(TypeInfo<ReturnTypeT>::GetIID((char*)templateClassName.c_str()), (void**)&ri))
            {                
                /* Didn't even support what we wanted, dispose of it and return null */
                obj->Release(); 
                return nullptr;
            }

            obj->Release(); // reduce reference count as 'obj' is going out of scope
            return ri; // ri should have a reference count of 1
        }
        else
        {
            // returning a plain object pointer type, force casted
            return (ReturnTypeT*)obj; // obj should have a reference count of 1
        }
    }

#define DECLARE_FACTORY_REGISTERED(factoryname, classname, via_interface) \
    private: \
    class IDMAPI RegistrationHookCaller\
    {\
    public:\
        RegistrationHookCaller()\
        {\
            classname::in_class_registration_hook();\
        }\
    };\
    static void in_class_registration_hook()\
    {\
        factoryname::getInstance()->Register(#classname, \
            []() { return (ISupports*)(via_interface*)(_new_ classname()); });\
    }\
    static RegistrationHookCaller registration_hook_caller; \
    virtual classname * Clone() { return new classname( *this ); }

#define IMPLEMENT_FACTORY_REGISTERED(classname) \
    classname::RegistrationHookCaller classname::registration_hook_caller;


// This definition will be gone eventually once we do campaign/event coordinator dll
#define DECLARE_FACTORY_REGISTERED_EXPORT(factoryname, classname, via_interface) \
    private: \
    class DTK_DLLEXPORT RegistrationHookCaller\
    {\
    public:\
        RegistrationHookCaller()\
        {\
            classname::in_class_registration_hook();\
        }\
    };\
    static void in_class_registration_hook()\
    {\
        factoryname::getInstance()->Register(#classname, \
            []() { return (ISupports*)(via_interface*)(_new_ classname()); });\
    }\
    static RegistrationHookCaller registration_hook_caller;
}
