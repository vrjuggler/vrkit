// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_MODE_COMPONENT_H_
#define _INF_MODE_COMPONENT_H_

#include <IOV/Config.h>

#include <string>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>

#include <IOV/ViewerPtr.h>
#include <IOV/ModeComponentPtr.h>

#define INF_MODE_COMPONENT_API_MAJOR    1
#define INF_MODE_COMPONENT_API_MINOR    0


namespace inf
{

/**
 * A plugin is an abstract interface that allows for an extension point in the
 * system.  It is used to add capabilities to the application.
 *
 * @since 0.23
 */
class IOV_CLASS_API ModeComponent
{
public:
   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static std::string getCreatorFuncName()
   {
      return "getCreator";
   }

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of inf::ModeComponent.
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
    *        valid dynamic code entity for an inf::ModeComponent instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

   virtual ~ModeComponent();

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
   virtual ModeComponentPtr init(inf::ViewerPtr viewer) = 0;

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
   bool activate(inf::ViewerPtr viewer);

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
   virtual void update(inf::ViewerPtr viewer) = 0;

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
   bool deactivate(inf::ViewerPtr viewer);

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
   void exit(inf::ViewerPtr viewer);

   /**
    * Returns the active state of this mode component.
    */
   bool isActive() const
   {
      return mActive;
   }

#if defined(VPR_OS_Windows)
   /**
    * Overlaod delete so that we can delete our memory correctly.  This is
    * necessary for DLLs on Windows to release memory from the correct memory
    * space.  All subclasses must overload delete similarly.
    */
   void operator delete(void* p)
   {
      if ( NULL != p )
      {
         ModeComponent* component_ptr = static_cast<ModeComponent*>(p);
         component_ptr->destroy();
      }
   }
#endif

protected:
   /**
    * Constructor.
    *
    * @post \c mActive is false.
    */
   ModeComponent();

   /**
    * Subclasses must implement this so that dynamically loaded plug-ins
    * delete themselves in the correct memory space.  This uses a template
    * pattern.
    */
   virtual void destroy() = 0;

   /**
    * Template method for activating this mode component. This is called
    * by activate(). Errors that prevent activation must be reported using
    * an exception that derives from std::exception.
    *
    * @pre init() has been invoked.
    *
    * @param viewer The viewer application object.
    *
    * @see inf::Exception
    * @see activate()
    */
   virtual void doActivate(inf::ViewerPtr viewer) = 0;

   /**
    * Template method for deactivating this mode component. This is called
    * by deactivate(). Errors that prevent deactivation must be reported using
    * an exception that derives from std::exception.
    *
    * @pre activate() has been invoked successfully.
    *
    * @param viewer The viewer application object.
    *
    * @see inf::Exception
    * @see deactivate()
    */
   virtual void doDeactivate(inf::ViewerPtr viewer) = 0;

   /**
    * Template method for de-initializing this mode component. This is called
    * by exit().
    *
    * @pre doDeactivate() has been invoked.
    *
    * @param viewer The viewer application object.
    */
   virtual void doExit(inf::ViewerPtr viewer) = 0;

private:
   bool mActive;        /**< The active state of this mode component. */
};

}


#endif /* _INF_MODE_COMPONENT_H_ */
