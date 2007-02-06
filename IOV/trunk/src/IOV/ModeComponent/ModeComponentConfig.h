// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_MODE_COMPONENT_CONFIG_H_
#define _INF_MODE_COMPONENT_CONFIG_H_

/*
 * ----------------------------------------------------------------------------
 * DLL-related macros.  These are based on the macros used by NSPR.  Use
 * IOV_MODE_COMPONENT_EXTERN for the prototype and IOV_IMPLEMENT for the implementation.
 * ----------------------------------------------------------------------------
 */
#if defined(WIN32) || defined(WIN64)

#   if defined(__GNUC__)
#       undef _declspec
#       define _declspec(x) __declspec(x)
#   endif

#   define IOV_MODE_COMPONENT_EXPORT(__type)      _declspec(dllexport) __type
#   define IOV_MODE_COMPONENT_EXPORT_CLASS        _declspec(dllexport)
#   define IOV_MODE_COMPONENT_EXPORT_DATA(__type) _declspec(dllexport) __type
#   define IOV_MODE_COMPONENT_IMPORT(__type)      _declspec(dllimport) __type
#   define IOV_MODE_COMPONENT_IMPORT_DATA(__type) _declspec(dllimport) __type
#   define IOV_MODE_COMPONENT_IMPORT_CLASS        _declspec(dllimport)

#   define IOV_MODE_COMPONENT_EXTERN(__type)         extern _declspec(dllexport) __type
#   define IOV_MODE_COMPONENT_IMPLEMENT(__type)      _declspec(dllexport) __type
#   define IOV_MODE_COMPONENT_EXTERN_DATA(__type)    extern _declspec(dllexport) __type
#   define IOV_MODE_COMPONENT_IMPLEMENT_DATA(__type) _declspec(dllexport) __type

#   define IOV_MODE_COMPONENT_CALLBACK
#   define IOV_MODE_COMPONENT_CALLBACK_DECL
#   define IOV_MODE_COMPONENT_STATIC_CALLBACK(__x) static __x

#else

#   define IOV_MODE_COMPONENT_EXPORT(__type)      __type
#   define IOV_MODE_COMPONENT_EXPORT_CLASS
#   define IOV_MODE_COMPONENT_EXPORT_DATA(__type) __type
#   define IOV_MODE_COMPONENT_IMPORT(__type)      __type
#   define IOV_MODE_COMPONENT_IMPORT_DATA(__type) __type
#   define IOV_MODE_COMPONENT_IMPORT_CLASS

#   define IOV_MODE_COMPONENT_EXTERN(__type)         extern __type
#   define IOV_MODE_COMPONENT_IMPLEMENT(__type)      __type
#   define IOV_MODE_COMPONENT_EXTERN_DATA(__type)    extern __type
#   define IOV_MODE_COMPONENT_IMPLEMENT_DATA(__type) __type

#   define IOV_MODE_COMPONENT_CALLBACK
#   define IOV_MODE_COMPONENT_CALLBACK_DECL
#   define IOV_MODE_COMPONENT_STATIC_CALLBACK(__x) static __x

#endif  /* WIN32 || WIN64 */

#ifdef _IOV_MODE_COMPONENT_BUILD_
#   define IOV_MODE_COMPONENT_API(__type)      IOV_MODE_COMPONENT_EXPORT(__type)
#   define IOV_MODE_COMPONENT_CLASS_API        IOV_MODE_COMPONENT_EXPORT_CLASS
#   define IOV_MODE_COMPONENT_DATA_API(__type) IOV_MODE_COMPONENT_EXPORT_DATA(__type)
#else
#   define IOV_MODE_COMPONENT_API(__type)      IOV_MODE_COMPONENT_IMPORT(__type)
#   define IOV_MODE_COMPONENT_CLASS_API        IOV_MODE_COMPONENT_IMPORT_CLASS
#   define IOV_MODE_COMPONENT_DATA_API(__type) IOV_MODE_COMPONENT_IMPORT_DATA(__type)
#endif

#endif /* _INF_MODE_COMPONENT_CONFIG_H_ */
