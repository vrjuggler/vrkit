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

#ifndef _VRKIT_GRAB_STRATEGY_H_
#define _VRKIT_GRAB_STRATEGY_H_

#include <vrkit/Config.h>

#include <vector>
#include <boost/function.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <vpr/DynLoad/Library.h>

#include <vrkit/AbstractPlugin.h>
#include <vrkit/SceneObjectPtr.h>
#include <vrkit/ViewerPtr.h>
#include <vrkit/signal/Proxy.h>

#include <vrkit/grab/StrategyPtr.h>

#define VRKIT_GRAB_STRATEGY_PLUGIN_API_MAJOR 2
#define VRKIT_GRAB_STRATEGY_PLUGIN_API_MINOR 1


namespace vrkit
{

namespace grab
{

/**
 * A pluggable strategy for determining which scene object(s) to grab.
 *
 * @note This class was renamed from vrkit::GrabStrategy in version 0.47.
 *
 * @since 0.30.0
 */
class VRKIT_CLASS_API Strategy : public AbstractPlugin
{
public:
   /**
    * @since 0.36
    */
   typedef boost::shared_ptr<Strategy> ptr_type;

protected:
   Strategy(const plugin::Info& info);

public:
   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static std::string getCreatorFuncName()
   {
      return "getGrabStrategyCreator";
   }

   virtual ~Strategy();

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of vrkit::grab::Strategy.
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
    *           a vrkit::MoveStrategy instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

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
   virtual StrategyPtr init(ViewerPtr viewer, grab_callback_t grabCallback,
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

}


#endif /* _VRKIT_GRAB_STRATEGY_H_ */
