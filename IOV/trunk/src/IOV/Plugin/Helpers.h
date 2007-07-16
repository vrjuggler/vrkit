// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_PLUGIN_HELPERS_H_
#define _INF_PLUGIN_HELPERS_H_

#include <IOV/Config.h>

#include <string>
#include <vector>

#include <vpr/DynLoad/Library.h>


namespace inf
{

namespace plugin
{

/**
 * Finds all the plug-in modules in the given search path. This is done using
 * vpr::LibraryFinder, so all files matching a platform-specific pattern will
 * be discovered and returned.
 *
 * @param searchPath A collection of directories to search for plug-in
 *                   modules (i.e., DLLs or shared libraries depending on the
 *                   platform-specific terminology).
 *
 * @note If IOV was compiled against the Visual C++ debug runtime on
 *       Windows, then the plug-in search also factors in that it needs to
 *       look for plug-ins whose names end with \c _d.dll. Otherwise, the
 *       search is only for files with a platform-specific extension (.so,
 *       .dylib, or .dll).
 *
 * @see getDefaultSearchPath()
 *
 * @since 0.36
 */
IOV_API(std::vector<vpr::LibraryPtr>)
findModules(const std::vector<std::string>& searchPath);

/**
 * Returns the root directory for IOV plug-ins. This is relative to the value
 * of the environment variable IOV_BASE_DIR. If IOV_BASE_DIR is not set, then
 * this function returns an empty string.
 *
 * @since 0.36
 */
IOV_API(std::string) getPluginBaseDir();

/**
 *
 * @since 0.36
 */
IOV_API(std::vector<std::string>)
buildSearchPath(const std::vector<std::string>& roots,
                const bool searchDebug = true);

/**
 *
 * @since 0.36
 */
IOV_API(std::vector<std::string>)
getDefaultSearchPath(const std::string& subdir = "",
                     const bool searchDebug = true);

/**
 * Constructs and returns an IOV plug-in search path made up of directories
 * that are the composition of the default root directories and the given
 * subdirectories. For example, if the \p subdirs vector contains two
 * subdirectory names "grab" and "isect", \p searchDebug is true, and the
 * IOV_BASE_DIR environment variable is set, then the search path returned
 * will be as much as the following:
 *    - \c plugins/grab/debug
 *    - \c plugins/grab
 *    - \c plugins/isect/debug
 *    - \c plugins/isect
 *    - \c IOV_BASE_DIR/lib/IOV/plugins/grab/debug
 *    - \c IOV_BASE_DIR/lib/IOV/plugins/grab
 *    - \c IOV_BASE_DIR/lib/IOV/plugins/isect/debug
 *    - \c IOV_BASE_DIR/lib/IOV/plugins/isect
 *
 * If \p searchDebug is false, then the \c debug subdirectories will not be
 * included in the returned search path.
 *
 * Directories that do not exist are not included in the search path. Thus,
 * if the current working directory has no \c plugins subdirectory and the
 * IOV_BASE_DIR environment variable is not set, this function will return
 * an empty vector.
 *
 * This function actually just prepares a collection of valid root
 * directories where plug-ins might be found. The real work of constructing a
 * search path such as the one shown above is done by buildSearchPath().
 * Therefore, the main purpose of this function is to ensure that all IOV
 * code uses the same set of root directories when looking for plug-in
 * modules. Calling buildSearchPath() directory is valid, but it is then the
 * responsibility of the calling code to determine what root directories will
 * be used.
 *
 * @since 0.36
 *
 * @see buildSearchPath()
 */
IOV_API(std::vector<std::string>)
getDefaultSearchPath(const std::vector<std::string>& subdirs,
                     const bool searchDebug = true);

}

}


#endif /* _INF_PLUGIN_HELPERS_H_ */
