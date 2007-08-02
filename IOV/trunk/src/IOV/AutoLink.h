// Copyright (C) Infiscape Corporation 2005-2007

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
