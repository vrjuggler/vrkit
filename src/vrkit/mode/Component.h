// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_MODE_COMPONENT_H_
#define _VRKIT_MODE_COMPONENT_H_

#include <vrkit/Config.h>

#include <string>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>

#include <vrkit/AbstractPlugin.h>
#include <vrkit/ViewerPtr.h>
#include <vrkit/mode/ComponentPtr.h>

#define VRKIT_MODE_COMPONENT_API_MAJOR  2
#define VRKIT_MODE_COMPONENT_API_MINOR  1


namespace vrkit
{

namespace mode
{

/** \class Component Component.h vrkit/mode/Component.h
 *
 * A plug-in is an abstract interface that allows for an extension point in
 * vrkit. It is used to add capabilities to the application. Mode components
 * specifically are intended to \em interpret events. For example, an object
 * intersection event could have two different interpretations by two
 * (mutually exclusive) mode components.
 *
 * @note This class was renamed from vrkit::ModeComponent in version 0.47.
 *
 * @since 0.23
 */
class VRKIT_CLASS_API Component : public AbstractPlugin
{
protected:
   /**
    * Constructor.
    *
    * @post \c mActive is false.
    */
   Component(const plugin::Info& info);

public:
   /**
    * @since 0.36
    */
   typedef boost::shared_ptr<Component> ptr_type;

   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static std::string getCreatorFuncName()
   {
      return "getCreator";
   }

   virtual ~Component();

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of vrkit::mode::Component.
    *
    * @pre \p pluginLib must have been loaded successfully.
    *
    * @param pluginLib The code entity (shared library, DLL, executable) to
    *                  validate.
    *
    * @return This method always returns true. If the plug-in is not valid,
    *         an exception is thrown.
    *
    * @throw vrkit::PluginInterfaceException
    *           Thrown if \p pluginLib is not a valid dynamic code entity for
    *           a vrkit::mode::Component instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

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
    *
    * @note Implementations of this method should not assume that \p viewer is
    *       fully initialized.
    */
   virtual ComponentPtr init(ViewerPtr viewer) = 0;

   /**
    * Attempts to activate this mode component by invoking the template
    * method doActivate(). If doActivate() throws an exception, then component
    * activation is considered to have failed.
    *
    * @pre init() has been invoked. \p viewer is fully initialized.
    * @post If activation succeeds, \c mActive is true.
    *
    * @param viewer The viewer application object.
    *
    * @return \c true is returned if this component was activated
    *         successfully. Otherwise, \c false is returned.
    *
    * @see doActivate()
    */
   bool activate(ViewerPtr viewer);

   /**
    * Tells this plug-in that it can perform its specific action(s) based on
    * its current state.  The state of this plug-in may or may not have been
    * updated depending on its focus, but this method will be invoked
    * regardless of the focus state.
    *
    * @pre This mode component is active.
    *
    * @see activate()
    */
   virtual void update(ViewerPtr viewer) = 0;

   /**
    * Attempts to deactivate this mode component by invoking the template
    * method doDeactivate(). If doActivate() throws an exception, then
    * component deactivation is considered to have failed.
    *
    * @pre This mode component is active.
    * @post If deactivation succeeds, \c mActive is false.
    *
    * @param viewer The viewer application object.
    *
    * @return \c true is returned if this component was deactivated
    *         successfully. Otherwise, \c false is returned.
    *
    * @see activate()
    * @see doDeactivate()
    */
   bool deactivate(ViewerPtr viewer);

   /**
    * Attempts to de-initialize this mode component by invoking the template
    * method doExit(). If this mode component is still active when this
    * method is invoked, then deactivate() is called before trying to
    * de-initialize.
    *
    * @post This mode component is de-initialized and cannot be used again
    *       without first calling init().
    *
    * @param viewer The viewer application object.
    *
    * @see isActive()
    * @see deactivate()
    * @see doExit()
    */
   void exit(ViewerPtr viewer);

   /**
    * Returns the active state of this mode component.
    */
   bool isActive() const
   {
      return mActive;
   }

   /**
    * Template method for activating this mode component. This is called
    * by activate(). Errors that prevent activation must be reported using
    * an exception that derives from std::exception.
    *
    * @pre init() has been invoked.
    *
    * @param viewer The viewer application object.
    *
    * @see vrkit::Exception
    * @see activate()
    */
   virtual void doActivate(ViewerPtr viewer) = 0;

   /**
    * Template method for deactivating this mode component. This is called
    * by deactivate(). Errors that prevent deactivation must be reported using
    * an exception that derives from std::exception.
    *
    * @pre activate() has been invoked successfully.
    *
    * @param viewer The viewer application object.
    *
    * @see vrkit::Exception
    * @see deactivate()
    */
   virtual void doDeactivate(ViewerPtr viewer) = 0;

   /**
    * Template method for de-initializing this mode component. This is called
    * by exit().
    *
    * @pre doDeactivate() has been invoked.
    *
    * @param viewer The viewer application object.
    */
   virtual void doExit(ViewerPtr viewer) = 0;

private:
   bool mActive;        /**< The active state of this mode component. */
};

}

}


#endif /* _VRKIT_MODE_COMPONENT_H_ */
