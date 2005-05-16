#ifndef PLUGIN_H
#define PLUGIN_H

#include <OpenSG/VRJ/Viewer/IOV/Config.h>

#include <string>
#include <jccl/Config/ConfigElementPtr.h>

#include <OpenSG/VRJ/Viewer/IOV/ViewerPtr.h>
#include <OpenSG/VRJ/Viewer/IOV/PluginPtr.h>

#define INF_PLUGIN_API_MAJOR    1
#define INF_PLUGIN_API_MINOR    0

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
   void setFocused(const bool focused)
   {
      if ( mIsFocused != focused )
      {
         mIsFocused = focused;
         focusChanged();
      }
   }

#ifdef WIN32
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

   virtual void focusChanged()
   {
      /* Do nothing. */ ;
   }

   bool mIsFocused;
};

}

#endif
