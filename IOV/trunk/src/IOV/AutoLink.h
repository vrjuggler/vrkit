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

#ifndef _IOV_AUTO_LINK_H_
#define _IOV_AUTO_LINK_H_

#if defined(WIN32) || defined(WIN64)
#  include <IOV/Version.h>

#  if defined(IOV_DEBUG) && defined(_DEBUG)
#     define IOV_LIB_RT_OPT "_d"
#  else
#     define IOV_LIB_RT_OPT ""
#  endif

#  pragma comment(lib, "iov" IOV_LIB_RT_OPT ".lib")

/*
#  if defined(IOV_DEBUG) && defined(_DEBUG)
#     define IOV_LIB_RT_OPT "_d"
#  else
#     define IOV_LIB_RT_OPT ""
#  endif

#  define IOV_STRINGIZE(X) IOV_DO_STRINGIZE(X)
#  define IOV_DO_STRINGIZE(X) #X
#  define IOV_VERSION_STR IOV_STRINGIZE(IOV_VERSION_MAJOR) "_" \
                          IOV_STRINGIZE(IOV_VERSION_MINOR) "_" \
                          IOV_STRINGIZE(IOV_VERSION_PATCH)

#  pragma comment(lib, "iov" IOV_LIB_RT_OPT "-" IOV_VERSION_STR ".lib")

#  undef IOV_VERSION_STR
#  undef IOV_DO_STRINGIZE
#  undef IOV_STRINGIZE
#  undef IOV_LIB_RT_OPT
*/
#endif

#endif /* _IOV_AUTO_LINK_H_ */
