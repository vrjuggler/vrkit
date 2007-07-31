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
 * Constructcs a search path using the given collection of root directories.
 * Each root directory is added to the search path only if it existed at the
 * time that this function was called.
 *
 * If there is a \c debug subdirectory of a root directory, it will be
 * included in the returned search path if and only if \p searchDebug is true.
 * The precedence of the \c debug subdirectory in the search path depends on
 * whether \c IOV_DEBUG was defined when this function was compiled. If it
 * was, then the \c debug subdirectory of the root directory is searched
 * before the root directory. If \c IOV_DEBUG was not defined, then the
 * \c debug subdirectory is search after its parent root directory.
 *
 * @param roots       The collection of directories that may be included in
 *                    the returned search path.
 * @param searchDebug Indicates whether the \c debug subdirectory should be
 *                    included in the search path (if it exists). This
 *                    parameter is optional, and it defaults to true if not
 *                    specified.
 *
 * @return A vector of strings representing directories where plug-ins may be
 *         found. Each string represents a directory that existed at the time
 *         that this function was called. This may or may not be the same as
 *         \p roots depending on the value of \p searchDebug and whether each
 *         root directory named in \p roots existed when this function was
 *         called.
 *
 * @since 0.36
 */
IOV_API(std::vector<std::string>)
buildSearchPath(const std::vector<std::string>& roots,
                const bool searchDebug = true);

/**
 * A convenience overlaod of getDefaultSearchPath() that takes zero or one
 * subdirectory names to search. See the other overload of this function for
 * the detailed description of how this function behaves.
 *
 * @param subdir      The subdirectory under \c plugins to include in the
 *                    returned search path. If this string is empty, then the
 *                    \c plugins directory itself is what will be added to the
 *                    search path. This parameter is optional, and it defaults
 *                    to an empty string if not specified.
 * @param searchDebug Indicates whether the \c debug subdirectory should be
 *                    included in the search path (if it exists). This
 *                    parameter is optional, and it defaults to true if not
 *                    specified.
 *
 * @return A vector of strings representing directories where plug-ins may be
 *         found. Each string represents a directory that existed at the time
 *         that this function was called. Returned paths constructed using the
 *         value of the IOV_BASE_DIR environment variable will be relative or
 *         absolute depending the value of the environment variable.
 *
 * @since 0.36
 */
IOV_API(std::vector<std::string>)
getDefaultSearchPath(const std::string& subdir = "",
                     const bool searchDebug = true);

/**
 * Constructs and returns an IOV plug-in search path made up of directories
 * that are the composition of the default root directories and the given
 * subdirectories. The order of the paths involves several factors to handle
 * the case of debug-enabled builds and optimized builds with some degree of
 * automation and convenience. There are two factors considered: whether
 * \c IOV_DEBUG was defined when the function body was compiled and the value
 * of \p searchDebug. Consider the following example for the purposes of
 * illustrating the behavior of this function.
 *
 * If the \p subdirs vector contains two subdirectory names "grab" and
 * "isect", \p searchDebug is true, \c IOV_DEBUG was defined when this
 * function was compiled, and the IOV_BASE_DIR environment variable is set,
 * then the search path returned will be as much as the following:
 *    - \c plugins/grab/debug
 *    - \c plugins/grab
 *    - \c plugins/isect/debug
 *    - \c plugins/isect
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/grab/debug
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/grab
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/isect/debug
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/isect
 *
 * If \c IOV_DEBUG was not defined and \p searchDebug is true, then the order
 * will be as much as the following:
 *    - \c plugins/grab
 *    - \c plugins/grab/debug
 *    - \c plugins/isect
 *    - \c plugins/isect/debug
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/grab
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/grab/debug
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/isect
 *    - \c $IOV_BASE_DIR/lib/IOV/plugins/isect/debug
 *
 * Note that the \c debug subdirectory is now searched second instead of
 * first. Thus, the debug-enabled state of the compiled IOV library determines
 * the precedence of the \c debug subdirectory in the search path. If
 * \p searchDebug is false (regardless of whether \c IOV_DEBUG was defined),
 * then the \c debug subdirectories will \em not be included in the returned
 * search path.
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
 * @param subdirs     The collection of subdirectory names under \c plugins to
 *                    be included in the returned search path. If this vector
 *                    is empty, then the \c plugins directory itself is what
 *                    will be added to the search path.
 * @param searchDebug Indicates whether the \c debug subdirectory should be
 *                    included in the search path (if it exists). This
 *                    parameter is optional, and it defaults to true if not
 *                    specified.
 *
 * @return A vector of strings representing directories where plug-ins may be
 *         found. Each string represents a directory that existed at the time
 *         that this function was called. Returned paths constructed using the
 *         value of the IOV_BASE_DIR environment variable will be relative or
 *         absolute depending the value of the environment variable.
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
