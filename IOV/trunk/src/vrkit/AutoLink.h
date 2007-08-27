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

#ifndef _VRKIT_AUTO_LINK_H_
#define _VRKIT_AUTO_LINK_H_

#if defined(WIN32) || defined(WIN64)
//#  include <vrkit/Version.h>

#  if defined(VRKIT_DEBUG) && defined(_DEBUG)
#     define VRKIT_LIB_RT_OPT "_d"
#  else
#     define VRKIT_LIB_RT_OPT ""
#  endif

#  pragma comment(lib, "vrkit" VRKIT_LIB_RT_OPT ".lib")

/*
#  if defined(VRKIT_DEBUG) && defined(_DEBUG)
#     define VRKIT_LIB_RT_OPT "_d"
#  else
#     define VRKIT_LIB_RT_OPT ""
#  endif

#  define VRKIT_STRINGIZE(X) VRKIT_DO_STRINGIZE(X)
#  define VRKIT_DO_STRINGIZE(X) #X
#  define VRKIT_VERSION_STR VRKIT_STRINGIZE(VRKIT_VERSION_MAJOR) "_" \
                            VRKIT_STRINGIZE(VRKIT_VERSION_MINOR) "_" \
                            VRKIT_STRINGIZE(VRKIT_VERSION_PATCH)

#  pragma comment(lib, "vrkit" VRKIT_LIB_RT_OPT "-" VRKIT_VERSION_STR ".lib")

#  undef VRKIT_VERSION_STR
#  undef VRKIT_DO_STRINGIZE
#  undef VRKIT_STRINGIZE
#  undef VRKIT_LIB_RT_OPT
*/
#endif

#endif /* _VRKIT_AUTO_LINK_H_ */
