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

#ifndef _VRKIT_VERSION_H_
#define _VRKIT_VERSION_H_

/**
 * This file contains two useful items.
 *    1. The preprocessor friendly VRKIT_VERSION "string". It is in the form
 *       <major><minor><patch> where each part has exactly 3 digits.
 *    2. The C++ friendly variable, version, that contains the version as a
 *       string. It is in the form of <major>.<minor>.<patch> where each part
 *       has anywhere from 1 to 3 digits.
 */

/**
 * This is the "human-readable" vrkit version _string_. It is of the form
 * <major><minor><patch>. Each part has exactly 3 digits.
 */
// The major/minor/patch version (up to 3 digits each).
#define VRKIT_VERSION_MAJOR    0
#define VRKIT_VERSION_MINOR    49
#define VRKIT_VERSION_PATCH    0

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// To update the version number, do not modify anything below this line!!!!
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Define the helper macros
//--------------------------------------------------------------------------

// These helper macros are used build up the VRKIT_VERSION macro
#define VRKIT_GLUE(a,b)        a ## b
#define VRKIT_XGLUE(a,b)       VRKIT_GLUE(a,b)

// These helper macros are used to stringify a given macro
#define VRKIT_STR(s)           # s
#define VRKIT_XSTR(s)          VRKIT_STR(s)

// These helper macros are used to build up the VRKIT_VERSION_STRING macro.
#define VRKIT_DOT(a,b)         a ## . ## b
#define VRKIT_XDOT(a,b)        VRKIT_DOT(a,b)

// These helpr macros are used to facilitate a zero left fill
#define VRKIT_ZEROFILL(a)      0 ## a
#define VRKIT_XZEROFILL(a)     VRKIT_ZEROFILL(a)

#if VRKIT_VERSION_MAJOR == 0
#  define VRKIT_VERSION_MAJOR_FILLED
#else
#  define VRKIT_VERSION_MAJOR_FILLED VRKIT_VERSION_MAJOR
#endif

// Fix up the minor version by doing a zero left fill
#if VRKIT_VERSION_MAJOR == 0
#  define VRKIT_VERSION_MINOR_FILLED VRKIT_VERSION_MINOR
#else
#  if VRKIT_VERSION_MINOR < 10
#     define VRKIT_VERSION_MINOR_FILLED \
                  VRKIT_XZEROFILL(VRKIT_XZEROFILL(VRKIT_VERSION_MINOR))
#  elif VRKIT_VERSION_MINOR < 100
#     define VRKIT_VERSION_MINOR_FILLED VRKIT_XZEROFILL(VRKIT_VERSION_MINOR)
#  else
#     define VRKIT_VERSION_MINOR_FILLED VRKIT_VERSION_MINOR
#  endif
#endif

// Fix up the patch version by doing a zero left fill
#if VRKIT_VERSION_PATCH < 10
#  define VRKIT_VERSION_PATCH_FILLED \
               VRKIT_XZEROFILL(VRKIT_XZEROFILL(VRKIT_VERSION_PATCH))
#elif VRKIT_VERSION_PATCH < 100
#  define VRKIT_VERSION_PATCH_FILLED VRKIT_XZEROFILL(VRKIT_VERSION_PATCH)
#else
#  define VRKIT_VERSION_PATCH_FILLED VRKIT_VERSION_PATCH
#endif

//--------------------------------------------------------------------------
// Define the VRKIT_VERSION and VRKIT_VERSION_STRING macros
//--------------------------------------------------------------------------

// Build up the VRKIT_VERSION macro by pasting the individual parts together
/**
 * The is the preprocessor-friendly version string. It is in the form of
 * <major><minor><patch>. Each part has exactly 3 digits.
 */
#define VRKIT_VERSION \
   VRKIT_XGLUE( \
      VRKIT_XGLUE(VRKIT_VERSION_MAJOR_FILLED, VRKIT_VERSION_MINOR_FILLED), \
      VRKIT_VERSION_PATCH_FILLED \
   )

// Create the VRKIT_VERSION_STRING macro
#define VRKIT_VERSION_STRING \
   VRKIT_XDOT( \
      VRKIT_XDOT(VRKIT_VERSION_MAJOR, VRKIT_VERSION_MINOR), \
      VRKIT_VERSION_PATCH \
   )

//--------------------------------------------------------------------------
// Declare a version string constant that can be used at runtime.
//--------------------------------------------------------------------------
namespace vrkit
{
   inline const char* getVersion()
   {
      return VRKIT_XSTR(VRKIT_VERSION_STRING);
   }
} // end namespace vrkit

//--------------------------------------------------------------------------
// Cleanup after ourselves and undef all internal macros.
//--------------------------------------------------------------------------

/*
// Undef the all helper macros
#undef VRKIT_XGLUE
#undef VRKIT_GLUE
#undef VRKIT_XSTR
#undef VRKIT_STR
#undef VRKIT_ZEROFILL
#undef VRKIT_XZEROFILL
#undef VRKIT_XDOT
#undef VRKIT_DOT

// Undef the VRKIT_VERSION_STRING temporary macro
#undef VRKIT_VERSION_STRING

// Undef the XXX_FILLED temporary macros
#undef VRKIT_VERSION_MAJOR_FILLED
#undef VRKIT_VERSION_MINOR_FILLED
#undef VRKIT_VERSION_PATCH_FILLED
*/

#endif
