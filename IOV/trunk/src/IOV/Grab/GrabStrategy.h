// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_GRAB_STRATEGY_H_
#define _INF_GRAB_STRATEGY_H_

#include <IOV/Config.h>

#include <vector>
#include <boost/function.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <vpr/DynLoad/Library.h>

#include <IOV/AbstractPlugin.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/ViewerPtr.h>
#include <IOV/Util/SignalProxy.h>

#include <IOV/Grab/GrabStrategyPtr.h>

#define INF_GRAB_STRATEGY_PLUGIN_API_MAJOR 2
#define INF_GRAB_STRATEGY_PLUGIN_API_MINOR 1


namespace inf
{

/**
 * A pluggable strategy for determining which scene object(s) to grab.
 *
 * @since 0.30.0
 */
class IOV_CLASS_API GrabStrategy : public AbstractPlugin
{
public:
   /**
    * @since 0.36
    */
   typedef boost::shared_ptr<GrabStrategy> ptr_type;

protected:
   GrabStrategy(const inf::plugin::Info& info);

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

   typedef boost::function<void (const std::vector<SceneObjectPtr>&,
                                 const gmtl::Point3f&)>
      grab_callback_t;

   typedef boost::function<void (const std::vector<SceneObjectPtr>&)>
      release_callback_t;

   /**
    * Initializes this grab strategy.
    *
    * @post This grab strategy is initialized and ready to be used.
    *
    * @param viewer          The VR Juggler application object.
    * @param grabCallback    A callback into code using this grab strategy
    *                        to be invoked when one or more grabbed objects
    *                        are grabbed.
    * @param releaseCallback A callback into code using this grab strategy
    *                        to be invoked when one or more grabbed objects
    *                        are released.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual GrabStrategyPtr init(ViewerPtr viewer, grab_callback_t grabCallback,
                                release_callback_t releaseCallback) = 0;

   /**
    * Indicates to this strategy that it has focus. This is a good place to
    * update displayed information about how object grabbing can be achieved.
    */
   virtual void setFocus(ViewerPtr viewer, const bool focused) = 0;

   /**
    * Checks for state changes based on input that could result in new objects
    * being grabbed. Any objects that have been grabbed since the last time
    * this method was invoked are returned to the caller.
    *
    * @pre This strategy is focused (setFocus() was invoked with a value of
    *      true).
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(ViewerPtr viewer) = 0;

   /**
    * Returns the current collection of grabbed objects.
    *
    * @pre update() has been invoked and has returned.
    */
   virtual std::vector<SceneObjectPtr> getGrabbedObjects() = 0;
};

}


#endif /* _INF_GRAB_STRATEGY_H_ */
