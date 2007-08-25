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

#include <vpr/DynLoad/Library.h>

#include <IOV/AbstractPlugin.h>
#include <IOV/ViewerPtr.h>
#include <IOV/PluginPtr.h>

#define INF_PLUGIN_API_MAJOR    2
#define INF_PLUGIN_API_MINOR    1


namespace inf
{

/**
 * A plugin is an abstract interface that allows for an extension point in the
 * system.  It is used to add capabilities to the application.
 */
class IOV_CLASS_API Plugin : public AbstractPlugin
{
public:
   /**
    * @since 0.36
    */
   typedef boost::shared_ptr<Plugin> ptr_type;

   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static const std::string getCreatorFuncName()
   {
      return "getCreator";
   }

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of inf::Plugin.
    *
    * @pre \p pluginLib must have been loaded successfully.
    *
    * @param pluginLib The code entity (shared library, DLL, executable) to
    *                  validate.
    *
    * @return This method always returns true. If the plug-in is not valid,
    *         an exception is thrown.
    *
    * @throw inf::PluginInterfaceException is thrown if \p pluginLib is not a
    *        valid dynamic code entity for an inf::Plugin instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

   virtual ~Plugin();

   /**
    * Returns a short (two- or three-word) description of this plug-in
    * suitable for being displayed to the application user.
    */
   virtual std::string getDescription() = 0;

   /**
    * Intializes this plug-in. This method is called as part of the setup in
    * the Viewer init method. This plug-in should read configuration from the
    * Viewer configuration object and take care of any setup needed.
    *
    * @param viewer The VR Juggler application object.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual PluginPtr init(inf::ViewerPtr viewer) = 0;

   /**
    * Perform context-specific activities when a context is opened. This
    * implementation does nothing.
    *
    * @pre vrj::OpenSGApp::contextInit() has been invoked.
    *
    * @param viewer The VR Juggler application object instance within which
    *               this plug-in is active. This parameter is ignored by
    *               this implementation.
    *
    * @since 0.28.0
    */
   virtual void contextInit(inf::ViewerPtr viewer);

   /**
    * Tells this plug-in that it can perform its specific action(s) based on
    * its current state. The implementation should take the focus of this
    * plug-in into account. That is, a plug-in that does not have focus should
    * not receive input or examine the state of input devices, but it may
    * still have some function to perform based on its current state.
    *
    * @param viewer The VR Juggler application object instance within which
    *               this plug-in is active.
    *
    * @see isFocused()
    * @see setFocused()
    *
    * @since 0.27.0
    *
    * @note In version 0.27.0, updateState() and run() were merged back into
    *       this single method, and the plug-in API changed from 1.1 to 1.2.
    *       Plug-ins updating to this change should block the contents of
    *       their old updateState() method in a test for being focused.
    */
   virtual void update(inf::ViewerPtr viewer) = 0;

   /**
    * Perform context-specific activities before the main rendering is
    * started. This implementation does nothing.
    *
    * @pre vrj::OpenSGApp::contextPreDraw() has been invoked.
    *
    * @param viewer The VR Juggler application object instance within which
    *               this plug-in is active. This parameter is ignored by
    *               this implementation.
    *
    * @since 0.28.0
    */
   virtual void contextPreDraw(inf::ViewerPtr viewer);

   /**
    * Perform context-specific rendering activities. This implementation does
    * nothing.
    *
    * @pre vrj::OpenSGApp::draw() has been invoked.
    *
    * @param viewer The VR Juggler application object instance within which
    *               this plug-in is active. This parameter is ignored by
    *               this implementation.
    *
    * @since 0.28.0
    */
   virtual void draw(inf::ViewerPtr viewer);

   /**
    * Perform context-specific activities after the main rendering has
    * completed. This implementation does nothing.
    *
    * @pre vrj::OpenSGApp::contextPostDraw() has been invoked.
    *
    * @param viewer The VR Juggler application object instance within which
    *               this plug-in is active. This parameter is ignored by
    *               this implementation.
    *
    * @since 0.28.0
    */
   virtual void contextPostDraw(inf::ViewerPtr viewer);

   /**
    * Perform context-specific activities when a context is closing. This
    * implementation does nothing.
    *
    * @pre vrj::OpenSGApp::contextClose() has been invoked.
    *
    * @param viewer The VR Juggler application object instance within which
    *               this plug-in is active. This parameter is ignored by
    *               this implementation.
    *
    * @since 0.28.0
    */
   virtual void contextClose(inf::ViewerPtr viewer);

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

protected:
   Plugin(const inf::plugin::Info& info);

   virtual void focusChanged(inf::ViewerPtr viewer)
   {
      /* Do nothing. */ ;
   }

   bool mIsFocused;              /**< If true, the plugin has "focus". */
};

}

#endif
