// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_DEBUG_H_
#define _INF_DEBUG_H_

#include <vpr/Util/Debug.h>


/** @name IOV Debug Categories */
//@{
/**
 * Debugging category for inf::Viewer.
 *
 * @since 0.26.4
 */
const vpr::DebugCategory infDBG_VIEWER(
   vpr::GUID("9bab82cf-2fb8-47dc-9f69-8a8343219ffc"), "DBG_INF_VIEWER",
   "infViewer:"
);

/**
 * Debugging category for inf::Plugin and subclasses thereof.
 *
 * @since 0.26.4
 */
const vpr::DebugCategory infDBG_PLUGIN(
   vpr::GUID("504a6441-6586-4bf2-acb9-73852d670f9c"), "DBG_INF_PLUGIN",
   "infPlugin:"
);
//@}


#endif /* _INF_DEBUG_H_ */
