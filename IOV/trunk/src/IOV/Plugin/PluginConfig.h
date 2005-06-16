// Copyright (C) Infiscape Corporation 2005

#ifndef _INF_PLUGIN_CONFIG_H_
#define _INF_PLUGIN_CONFIG_H_

/*
 * ----------------------------------------------------------------------------
 * DLL-related macros.  These are based on the macros used by NSPR.  Use
 * IOV_PLUGIN_EXTERN for the prototype and IOV_IMPLEMENT for the implementation.
 * ----------------------------------------------------------------------------
 */
#if defined(WIN32) || defined(WIN64)

#   if defined(__GNUC__)
#       undef _declspec
#       define _declspec(x) __declspec(x)
#   endif

#   define IOV_PLUGIN_EXPORT(__type)      _declspec(dllexport) __type
#   define IOV_PLUGIN_EXPORT_CLASS        _declspec(dllexport)
#   define IOV_PLUGIN_EXPORT_DATA(__type) _declspec(dllexport) __type
#   define IOV_PLUGIN_IMPORT(__type)      _declspec(dllimport) __type
#   define IOV_PLUGIN_IMPORT_DATA(__type) _declspec(dllimport) __type
#   define IOV_PLUGIN_IMPORT_CLASS        _declspec(dllimport)

#   define IOV_PLUGIN_EXTERN(__type)         extern _declspec(dllexport) __type
#   define IOV_PLUGIN_IMPLEMENT(__type)      _declspec(dllexport) __type
#   define IOV_PLUGIN_EXTERN_DATA(__type)    extern _declspec(dllexport) __type
#   define IOV_PLUGIN_IMPLEMENT_DATA(__type) _declspec(dllexport) __type

#   define IOV_PLUGIN_CALLBACK
#   define IOV_PLUGIN_CALLBACK_DECL
#   define IOV_PLUGIN_STATIC_CALLBACK(__x) static __x

#else

#   define IOV_PLUGIN_EXPORT(__type)      __type
#   define IOV_PLUGIN_EXPORT_CLASS
#   define IOV_PLUGIN_EXPORT_DATA(__type) __type
#   define IOV_PLUGIN_IMPORT(__type)      __type
#   define IOV_PLUGIN_IMPORT_DATA(__type) __type
#   define IOV_PLUGIN_IMPORT_CLASS

#   define IOV_PLUGIN_EXTERN(__type)         extern __type
#   define IOV_PLUGIN_IMPLEMENT(__type)      __type
#   define IOV_PLUGIN_EXTERN_DATA(__type)    extern __type
#   define IOV_PLUGIN_IMPLEMENT_DATA(__type) __type

#   define IOV_PLUGIN_CALLBACK
#   define IOV_PLUGIN_CALLBACK_DECL
#   define IOV_PLUGIN_STATIC_CALLBACK(__x) static __x

#endif  /* WIN32 || WIN64 */

#ifdef _IOV_PLUGIN_BUILD_
#   define IOV_PLUGIN_API(__type)      IOV_PLUGIN_EXPORT(__type)
#   define IOV_PLUGIN_CLASS_API        IOV_PLUGIN_EXPORT_CLASS
#   define IOV_PLUGIN_DATA_API(__type) IOV_PLUGIN_EXPORT_DATA(__type)
#else
#   define IOV_PLUGIN_API(__type)      IOV_PLUGIN_IMPORT(__type)
#   define IOV_PLUGIN_CLASS_API        IOV_PLUGIN_IMPORT_CLASS
#   define IOV_PLUGIN_DATA_API(__type) IOV_PLUGIN_IMPORT_DATA(__type)
#endif

#endif
