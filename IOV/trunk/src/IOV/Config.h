// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_CONFIG_H_
#define _INF_CONFIG_H_

/*
 * ----------------------------------------------------------------------------
 * DLL-related macros.  These are based on the macros used by NSPR.  Use
 * IOV_EXTERN for the prototype and IOV_IMPLEMENT for the implementation.
 * ----------------------------------------------------------------------------
 */
#if defined(WIN32) || defined(WIN64)

#   if defined(__GNUC__)
#       undef _declspec
#       define _declspec(x) __declspec(x)
#   endif

#   define IOV_EXPORT(__type)      _declspec(dllexport) __type
#   define IOV_EXPORT_CLASS        _declspec(dllexport)
#   define IOV_EXPORT_DATA(__type) _declspec(dllexport) __type
#   define IOV_IMPORT(__type)      _declspec(dllimport) __type
#   define IOV_IMPORT_DATA(__type) _declspec(dllimport) __type
#   define IOV_IMPORT_CLASS        _declspec(dllimport)

#   define IOV_EXTERN(__type)         extern _declspec(dllexport) __type
#   define IOV_IMPLEMENT(__type)      _declspec(dllexport) __type
#   define IOV_EXTERN_DATA(__type)    extern _declspec(dllexport) __type
#   define IOV_IMPLEMENT_DATA(__type) _declspec(dllexport) __type

#   define IOV_CALLBACK
#   define IOV_CALLBACK_DECL
#   define IOV_STATIC_CALLBACK(__x) static __x

#else

#   define IOV_EXPORT(__type)      __type
#   define IOV_EXPORT_CLASS
#   define IOV_EXPORT_DATA(__type) __type
#   define IOV_IMPORT(__type)      __type
#   define IOV_IMPORT_DATA(__type) __type
#   define IOV_IMPORT_CLASS

#   define IOV_EXTERN(__type)         extern __type
#   define IOV_IMPLEMENT(__type)      __type
#   define IOV_EXTERN_DATA(__type)    extern __type
#   define IOV_IMPLEMENT_DATA(__type) __type

#   define IOV_CALLBACK
#   define IOV_CALLBACK_DECL
#   define IOV_STATIC_CALLBACK(__x) static __x

#endif  /* WIN32 || WIN64 */

#ifdef _IOV_BUILD_
#   define IOV_API(__type)      IOV_EXPORT(__type)
#   define IOV_CLASS_API        IOV_EXPORT_CLASS
#   define IOV_DATA_API(__type) IOV_EXPORT_DATA(__type)
#else
#   define IOV_API(__type)      IOV_IMPORT(__type)
#   define IOV_CLASS_API        IOV_IMPORT_CLASS
#   define IOV_DATA_API(__type) IOV_IMPORT_DATA(__type)
#endif

#if ! defined(_IOV_BUILD_)
#  include <IOV/AutoLink.h>
#endif

#endif
