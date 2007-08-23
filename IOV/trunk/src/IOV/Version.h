// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VERSION_H_
#define _INF_VERSION_H_

/**
 * This file contains two useful items.
 *    1. The preprocessor friendly IOV_VERSION "string". It is in the form
 *       <major><minor><patch> where each part has exactly 3 digits.
 *    2. The C++ friendly variable, version, that contains the version as a
 *       string. It is in the form of <major>.<minor>.<patch> where each part
 *       has anywhere from 1 to 3 digits.
 */

/**
 * This is the "human-readable" IOV version _string_. It is of the form
 * <major><minor><patch>. Each part has exactly 3 digits.
 */
// The major/minor/patch version (up to 3 digits each).
#define IOV_VERSION_MAJOR    0
#define IOV_VERSION_MINOR    45
#define IOV_VERSION_PATCH    2

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// To update the version number, do not modify anything below this line!!!!
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Define the helper macros
//--------------------------------------------------------------------------

// These helper macros are used build up the IOV_VERSION macro
#define IOV_GLUE(a,b)        a ## b
#define IOV_XGLUE(a,b)       IOV_GLUE(a,b)

// These helper macros are used to stringify a given macro
#define IOV_STR(s)           # s
#define IOV_XSTR(s)          IOV_STR(s)

// These helper macros are used to build up the IOV_VERSION_STRING macro.
#define IOV_DOT(a,b)         a ## . ## b
#define IOV_XDOT(a,b)        IOV_DOT(a,b)

// These helpr macros are used to facilitate a zero left fill
#define IOV_ZEROFILL(a)      0 ## a
#define IOV_XZEROFILL(a)     IOV_ZEROFILL(a)

#if IOV_VERSION_MAJOR == 0
#  define IOV_VERSION_MAJOR_FILLED
#else
#  define IOV_VERSION_MAJOR_FILLED IOV_VERSION_MAJOR
#endif

// Fix up the minor version by doing a zero left fill
#if IOV_VERSION_MAJOR == 0
#  define IOV_VERSION_MINOR_FILLED IOV_VERSION_MINOR
#else
#  if IOV_VERSION_MINOR < 10
#     define IOV_VERSION_MINOR_FILLED \
                  IOV_XZEROFILL(IOV_XZEROFILL(IOV_VERSION_MINOR))
#  elif IOV_VERSION_MINOR < 100
#     define IOV_VERSION_MINOR_FILLED IOV_XZEROFILL(IOV_VERSION_MINOR)
#  else
#     define IOV_VERSION_MINOR_FILLED IOV_VERSION_MINOR
#  endif
#endif

// Fix up the patch version by doing a zero left fill
#if IOV_VERSION_PATCH < 10
#  define IOV_VERSION_PATCH_FILLED \
               IOV_XZEROFILL(IOV_XZEROFILL(IOV_VERSION_PATCH))
#elif IOV_VERSION_PATCH < 100
#  define IOV_VERSION_PATCH_FILLED IOV_XZEROFILL(IOV_VERSION_PATCH)
#else
#  define IOV_VERSION_PATCH_FILLED IOV_VERSION_PATCH
#endif

//--------------------------------------------------------------------------
// Define the IOV_VERSION and IOV_VERSION_STRING macros
//--------------------------------------------------------------------------

// Build up the IOV_VERSION macro by pasting the individual parts together
/**
 * The is the preprocessor-friendly version string. It is in the form of
 * <major><minor><patch>. Each part has exactly 3 digits.
 */
#define IOV_VERSION \
   IOV_XGLUE( \
      IOV_XGLUE(IOV_VERSION_MAJOR_FILLED, IOV_VERSION_MINOR_FILLED), \
      IOV_VERSION_PATCH_FILLED \
   )

// Create the IOV_VERSION_STRING macro
#define IOV_VERSION_STRING \
   IOV_XDOT( \
      IOV_XDOT(IOV_VERSION_MAJOR, IOV_VERSION_MINOR), \
      IOV_VERSION_PATCH \
   )

//--------------------------------------------------------------------------
// Declare a version string constant that can be used at runtime.
//--------------------------------------------------------------------------
namespace inf
{
   inline const char* getVersion()
   {
      return IOV_XSTR(IOV_VERSION_STRING);
   }
} // end namespace inf

//--------------------------------------------------------------------------
// Cleanup after ourselves and undef all internal macros.
//--------------------------------------------------------------------------

/*
// Undef the all helper macros
#undef IOV_XGLUE
#undef IOV_GLUE
#undef IOV_XSTR
#undef IOV_STR
#undef IOV_ZEROFILL
#undef IOV_XZEROFILL
#undef IOV_XDOT
#undef IOV_DOT

// Undef the IOV_VERSION_STRING temporary macro
#undef IOV_VERSION_STRING

// Undef the XXX_FILLED temporary macros
#undef IOV_VERSION_MAJOR_FILLED
#undef IOV_VERSION_MINOR_FILLED
#undef IOV_VERSION_PATCH_FILLED
*/

#endif
