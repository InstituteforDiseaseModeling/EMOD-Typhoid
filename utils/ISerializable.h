#pragma once

#include "ISupports.h"

#include <stack>        // for _pool declaration/implementation
#include "IArchive.h"   // for serialization declaration

namespace Kernel {
    struct IDMAPI ISerializable : ISupports
    {
        virtual const char* GetClassName() { std::cout << "GetClassName() not implemented on this class." << std::endl; std::cout.flush(); throw; }
        virtual void Recycle() { std::cout << "Recycle() not implemented on this class." << std::endl; std::cout.flush(); throw; }
        virtual ~ISerializable() {}
    };

    template <typename I>
    struct SerializationRegistrar
    {
        typedef void (*serialize_function_t)(IArchive&, I*);
        typedef I* (*constructor_function_t)(void);

        static void _register(const char* classname, serialize_function_t serializer, constructor_function_t constructor)
        {
            if (!_singleton) _singleton = new SerializationRegistrar<I>();
            _singleton->serializer_map[classname] = serializer;
            _singleton->constructor_map[classname] = constructor;
        }

        static serialize_function_t _get_serializer(std::string& classname) { return _singleton->serializer_map[classname]; }
        static constructor_function_t _get_constructor(std::string& classname) { return _singleton->constructor_map[classname]; }
        std::map<std::string, serialize_function_t> serializer_map;
        std::map<std::string, constructor_function_t> constructor_map;
        static SerializationRegistrar<I>* _singleton;
    };

    template<typename Derived, typename Interface>
    struct SerializationRegistrationCaller
    {
        SerializationRegistrationCaller()
        {
            in_class_registration_hook();
        }
        static void in_class_registration_hook()
        {
            typename SerializationRegistrar<Interface>::_register(
                typename Derived::_class_name.c_str(),
                typename Derived::serialize,
                typename Derived::construct);
        }
    };

    template<typename T>
    struct PoolManager
    {
        static T* _allocate()
        {
            T* _new;
            if (_pool.size() > 0)
            {
                _new = new(_pool.top()) T();
                _pool.pop();
            }
            else
                _new = new T();
            return _new;
        }
        static void _recycle(T* t)
        {
            t->~T();
            _pool.push(t);
        }
    private:
        static std::stack<T*> _pool;
    };

#define DECLARE_SERIALIZABLE(classname, base_interface)                             \
    private:                                                                        \
        virtual const char* GetClassName() override { return _class_name.c_str(); } \
        static std::string _class_name;                                             \
        friend SerializationRegistrationCaller<classname, base_interface>;          \
        static SerializationRegistrationCaller<classname, base_interface> serialization_registration_caller; \
        friend PoolManager<classname>; \
        static base_interface* construct() { return dynamic_cast<base_interface*>(PoolManager<classname>::_allocate()); }    \
        virtual void Recycle() override { PoolManager<classname>::_recycle(this); } \
    protected:                                              \
        static void serialize(IArchive&, base_interface*);  \


#define REGISTER_SERIALIZABLE(classname, base_interface)                                            \
    std::string classname::_class_name(#classname);                                                 \
    SerializationRegistrationCaller<classname, base_interface> classname::serialization_registration_caller; \
    std::stack<classname*> PoolManager<classname>::_pool;                                           \


#define DECLARE_SERIALIZATION_REGISTRAR(base_interface)         \
    static SerializationRegistrar<base_interface> _registrar;   \
    friend void serialize(IArchive&, base_interface*&);         \


    template<typename IF>
    void serialize(IArchive&ar, IF*& iface)
    {
        std::string class_name = ar.IsWriter() ? iface->GetClassName() : "__UNK__";
        ar.startElement();
        ar.labelElement("__cname__") & class_name;
        typename SerializationRegistrar<IF>::constructor_function_t constructor_function = SerializationRegistrar<IF>::_get_constructor(class_name);
        typename SerializationRegistrar<IF>::serialize_function_t serialize_function = SerializationRegistrar<IF>::_get_serializer(class_name);
        ar.labelElement("__inst__");
        if (!ar.IsWriter()) iface = constructor_function();
        serialize_function(ar, iface);
        ar.endElement();
    }

#define IMPLEMENT_SERIALIZATION_REGISTRAR(base_interface) \
    SerializationRegistrar<base_interface> base_interface::_registrar; \
    template void serialize<base_interface>(IArchive& ar, base_interface*& iface); \
    SerializationRegistrar<base_interface>* SerializationRegistrar<base_interface>::_singleton = nullptr; \

}
