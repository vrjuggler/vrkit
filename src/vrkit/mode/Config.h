// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_MODE_COMPONENT_CONFIG_H_
#define _VRKIT_MODE_COMPONENT_CONFIG_H_

/*
 * ----------------------------------------------------------------------------
 * DLL-related macros.  These are based on the macros used by NSPR.  Use
 * VRKIT_MODE_COMPONENT_EXTERN for the prototype and VRKIT_IMPLEMENT for the
 * implementation.
 * ----------------------------------------------------------------------------
 */
#if defined(WIN32) || defined(WIN64)

#   if defined(__GNUC__)
#       undef _declspec
#       define _declspec(x) __declspec(x)
#   endif

#   define VRKIT_MODE_COMPONENT_EXPORT(__type)      _declspec(dllexport) __type
#   define VRKIT_MODE_COMPONENT_EXPORT_CLASS        _declspec(dllexport)
#   define VRKIT_MODE_COMPONENT_EXPORT_DATA(__type) _declspec(dllexport) __type
#   define VRKIT_MODE_COMPONENT_IMPORT(__type)      _declspec(dllimport) __type
#   define VRKIT_MODE_COMPONENT_IMPORT_DATA(__type) _declspec(dllimport) __type
#   define VRKIT_MODE_COMPONENT_IMPORT_CLASS        _declspec(dllimport)

#   define VRKIT_MODE_COMPONENT_EXTERN(__type)         extern _declspec(dllexport) __type
#   define VRKIT_MODE_COMPONENT_IMPLEMENT(__type)      _declspec(dllexport) __type
#   define VRKIT_MODE_COMPONENT_EXTERN_DATA(__type)    extern _declspec(dllexport) __type
#   define VRKIT_MODE_COMPONENT_IMPLEMENT_DATA(__type) _declspec(dllexport) __type

#   define VRKIT_MODE_COMPONENT_CALLBACK
#   define VRKIT_MODE_COMPONENT_CALLBACK_DECL
#   define VRKIT_MODE_COMPONENT_STATIC_CALLBACK(__x) static __x

#else

#   define VRKIT_MODE_COMPONENT_EXPORT(__type)      __type
#   define VRKIT_MODE_COMPONENT_EXPORT_CLASS
#   define VRKIT_MODE_COMPONENT_EXPORT_DATA(__type) __type
#   define VRKIT_MODE_COMPONENT_IMPORT(__type)      __type
#   define VRKIT_MODE_COMPONENT_IMPORT_DATA(__type) __type
#   define VRKIT_MODE_COMPONENT_IMPORT_CLASS

#   define VRKIT_MODE_COMPONENT_EXTERN(__type)         extern __type
#   define VRKIT_MODE_COMPONENT_IMPLEMENT(__type)      __type
#   define VRKIT_MODE_COMPONENT_EXTERN_DATA(__type)    extern __type
#   define VRKIT_MODE_COMPONENT_IMPLEMENT_DATA(__type) __type

#   define VRKIT_MODE_COMPONENT_CALLBACK
#   define VRKIT_MODE_COMPONENT_CALLBACK_DECL
#   define VRKIT_MODE_COMPONENT_STATIC_CALLBACK(__x) static __x

#endif  /* WIN32 || WIN64 */

#ifdef _VRKIT_MODE_COMPONENT_BUILD_
#   define VRKIT_MODE_COMPONENT_API(__type)      VRKIT_MODE_COMPONENT_EXPORT(__type)
#   define VRKIT_MODE_COMPONENT_CLASS_API        VRKIT_MODE_COMPONENT_EXPORT_CLASS
#   define VRKIT_MODE_COMPONENT_DATA_API(__type) VRKIT_MODE_COMPONENT_EXPORT_DATA(__type)
#else
#   define VRKIT_MODE_COMPONENT_API(__type)      VRKIT_MODE_COMPONENT_IMPORT(__type)
#   define VRKIT_MODE_COMPONENT_CLASS_API        VRKIT_MODE_COMPONENT_IMPORT_CLASS
#   define VRKIT_MODE_COMPONENT_DATA_API(__type) VRKIT_MODE_COMPONENT_IMPORT_DATA(__type)
#endif

#endif /* _VRKIT_MODE_COMPONENT_CONFIG_H_ */
