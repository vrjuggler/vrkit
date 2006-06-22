#ifndef _INF_GRAB_STRATEGY_H_
#define _INF_GRAB_STRATEGY_H_

#include <IOV/Config.h>

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <vpr/DynLoad/Library.h>

#include <IOV/SceneObjectPtr.h>
#include <IOV/ViewerPtr.h>
#include <IOV/Util/SignalProxy.h>

#include <IOV/Grab/GrabStrategyPtr.h>

#define INF_GRAB_STRATEGY_PLUGIN_API_MAJOR 1
#define INF_GRAB_STRATEGY_PLUGIN_API_MINOR 0


namespace inf
{

/**
 * A pluggable strategy for determining which scene object(s) to grab.
 *
 * @since 0.30.0
 */
class IOV_CLASS_API GrabStrategy : boost::noncopyable
{
protected:
   GrabStrategy();

public:
   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static std::string getCreatorFuncName()
   {
      return "getGrabStrategyCreator";
   }

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of inf::GrabStrategy.
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
    *        valid dynamic code entity for an inf::MoveStrategy instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

   virtual ~GrabStrategy();

   /**
    * Initializes this grab strategy.
    *
    * @post This grab strategy is initialized and ready to be used.
    *
    * @param viewer The VR Juggler application object.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual GrabStrategyPtr init(ViewerPtr viewer) = 0;

   /**
    * Indicates to this strategy that it has focus. This is a good place to
    * update displayed information about how object grabbing can be achieved.
    */
   virtual void setFocus(ViewerPtr viewer, const bool focused) = 0;

   typedef boost::function<void (const std::vector<SceneObjectPtr>&,
                                 const gmtl::Point3f&)>
      grab_callback_t;

   typedef boost::function<void (const std::vector<SceneObjectPtr>&)>
      release_callback_t;

   /**
    * Checks for state changes based on input that could result in new objects
    * being grabbed. Any objects that have been grabbed since the last time
    * this method was invoked are returned to the caller.
    *
    * @pre This strategy is focused (setFocus() was invoked with a value of
    *      true).
    *
    * @param viewer         The VR Juggler application object.
    */
   virtual void update(ViewerPtr viewer, grab_callback_t grabCallback,
                       release_callback_t releaseCallback) = 0;

#if defined(WIN32) || defined(WIN64)
   /**
    * Overlaod delete so that we can delete our memory correctly. This is
    * necessary for DLLs on Windows to release memory from the correct memory
    * space. All subclasses must overload delete similarly.
    */
   void operator delete(void* p)
   {
      if ( NULL != p )
      {
         GrabStrategy* strategy_ptr = static_cast<GrabStrategy*>(p);
         strategy_ptr->destroy();
      }
   }
#endif

protected:
   /**
    * Subclasses must implement this so that dynamically loaded plug-ins
    * delete themselves in the correct memory space. This uses the Template
    * Method Pattern.
    */
   virtual void destroy() = 0;
};

}


#endif /* _INF_GRAB_STRATEGY_H_ */
