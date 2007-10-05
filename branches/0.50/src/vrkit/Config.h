// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _VRKIT_CONFIG_H_
#define _VRKIT_CONFIG_H_

/*
 * ----------------------------------------------------------------------------
 * DLL-related macros.  These are based on the macros used by NSPR.  Use
 * VRKIT_EXTERN for the prototype and VRKIT_IMPLEMENT for the implementation.
 * ----------------------------------------------------------------------------
 */
#if defined(WIN32) || defined(WIN64)

#   if defined(__GNUC__)
#       undef _declspec
#       define _declspec(x) __declspec(x)
#   endif

#   define VRKIT_EXPORT(__type)      _declspec(dllexport) __type
#   define VRKIT_EXPORT_CLASS        _declspec(dllexport)
#   define VRKIT_EXPORT_DATA(__type) _declspec(dllexport) __type
#   define VRKIT_IMPORT(__type)      _declspec(dllimport) __type
#   define VRKIT_IMPORT_DATA(__type) _declspec(dllimport) __type
#   define VRKIT_IMPORT_CLASS        _declspec(dllimport)

#   define VRKIT_EXTERN(__type)         extern _declspec(dllexport) __type
#   define VRKIT_IMPLEMENT(__type)      _declspec(dllexport) __type
#   define VRKIT_EXTERN_DATA(__type)    extern _declspec(dllexport) __type
#   define VRKIT_IMPLEMENT_DATA(__type) _declspec(dllexport) __type

#   define VRKIT_CALLBACK
#   define VRKIT_CALLBACK_DECL
#   define VRKIT_STATIC_CALLBACK(__x) static __x

#else

#   define VRKIT_EXPORT(__type)      __type
#   define VRKIT_EXPORT_CLASS
#   define VRKIT_EXPORT_DATA(__type) __type
#   define VRKIT_IMPORT(__type)      __type
#   define VRKIT_IMPORT_DATA(__type) __type
#   define VRKIT_IMPORT_CLASS

#   define VRKIT_EXTERN(__type)         extern __type
#   define VRKIT_IMPLEMENT(__type)      __type
#   define VRKIT_EXTERN_DATA(__type)    extern __type
#   define VRKIT_IMPLEMENT_DATA(__type) __type

#   define VRKIT_CALLBACK
#   define VRKIT_CALLBACK_DECL
#   define VRKIT_STATIC_CALLBACK(__x) static __x

#endif  /* WIN32 || WIN64 */

#ifdef _VRKIT_BUILD_
#   define VRKIT_API(__type)      VRKIT_EXPORT(__type)
#   define VRKIT_CLASS_API        VRKIT_EXPORT_CLASS
#   define VRKIT_DATA_API(__type) VRKIT_EXPORT_DATA(__type)
#else
#   define VRKIT_API(__type)      VRKIT_IMPORT(__type)
#   define VRKIT_CLASS_API        VRKIT_IMPORT_CLASS
#   define VRKIT_DATA_API(__type) VRKIT_IMPORT_DATA(__type)
#endif

#if ! defined(_VRKIT_BUILD_)
#  include <vrkit/AutoLink.h>
#endif

#endif /* _VRKIT_CONFIG_H_ */
