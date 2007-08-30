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

#ifndef _INF_PLUGIN_H_
#define _INF_PLUGIN_H_

#include <IOV/Config.h>

#include <string>

#include <IOV/ViewerPtr.h>
#include <IOV/PluginPtr.h>

#define INF_PLUGIN_API_MAJOR    1
#define INF_PLUGIN_API_MINOR    1

namespace inf
{

/**
 * A plugin is an abstract interface that allows for an extension point in the
 * system.  It is used to add capabilities to the application.
 */
class IOV_CLASS_API Plugin
{
public:
   virtual ~Plugin();

   /**
    * Returns a short (two- or three-word) description of this plug-in
    * suitable for being displayed to the application user.
    */
   virtual std::string getDescription() = 0;

   /** Intialize the plugin.
    * This method is called as part of the setup in the Viewer init method.
    * The plugin should read configuration from the Viewer configuration object
    * and take care of any setup needed.
    */
   virtual void init(inf::ViewerPtr viewer) = 0;

   /**
    * Tells this plug-in to update its state, which generally means that it
    * should do whatever it needs to do before run() is invoked.
    *
    * @pre This plug-in has focus.
    */
   virtual void updateState(inf::ViewerPtr viewer) = 0;

   /**
    * Tells this plug-in that it can perform its specific action(s) based on
    * its current state.  The state of this plug-in may or may not have been
    * updated depending on its focus, but this method will be invoked
    * regardless of the focus state.
    *
    * @see updateState(), setFocused()
    */
   virtual void run(inf::ViewerPtr viewer) = 0;

   bool isFocused() const
   {
      return mIsFocused;
   }

   /**
    * Changes the focus state of this plug-in.
    */
   void setFocused(inf::ViewerPtr viewer, const bool focused)
   {
      if ( mIsFocused != focused )
      {
         mIsFocused = focused;
         focusChanged(viewer);
      }
   }

#if defined(WIN32) || defined(WIN64)
   /**
    * Overlaod delete so that we can delete our memory correctly.  This is
    * necessary for DLLs on Win32 to release memory from the correct memory
    * space.  All subclasses must overload delete similarly.
    */
   void operator delete(void* p)
   {
      if ( NULL != p )
      {
         Plugin* plugin_ptr = static_cast<Plugin*>(p);
         plugin_ptr->destroy();
      }
   }
#endif

protected:
   /**
    * Subclasses must implement this so that dynamically loaded plug-ins
    * delete themselves in the correct memory space.  This uses a template
    * pattern.
    */
   virtual void destroy() = 0;

   Plugin();

   virtual void focusChanged(inf::ViewerPtr viewer)
   {
      /* Do nothing. */ ;
   }

   bool mIsFocused;              /**< If true, the plugin has "focus". */
};

}

#endif
