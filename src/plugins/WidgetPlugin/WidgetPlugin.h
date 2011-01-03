// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_WIDGET_PLUGIN_H_
#define _VRKIT_WIDGET_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <gmtl/Matrix.h>
#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/WandInterfacePtr.h>
#include <vrkit/WidgetPtr.h>
#include <vrkit/scenedata/Event.h>
#include <vrkit/scenedata/WidgetDataPtr.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/viewer/Plugin.h>
#include <vrkit/isect/Strategy.h>
#include <vrkit/util/DigitalCommand.h>


namespace vrkit
{

class WidgetPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<WidgetPlugin>
{
protected:
   WidgetPlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new WidgetPlugin(info));
   }

   virtual ~WidgetPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Widget Plug-in");
   }

   virtual viewer::PluginPtr init(ViewerPtr viewer);

   /**
    * Updates the state of the widgets based on input from the wand interface.
    *
    * @post The state of all widgets is updated to reflect the digtial and/or
    *       analog input from the user via the wand interface.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(ViewerPtr viewer);

protected:
   /**
    * Updates the state of the status panel (if it is in use) based on the
    * new focused state of this plug-in.
    *
    * @post If this plug-in has lost focus, \c mSelectedGridIndex will be -1.
    *       This means that no grid will be selected the next time that this
    *       plug-in is activated. If one or more grids are visible when focus
    *       is lost, they remain visible, but none are selected.
    *
    * @param viewer The VR Juggler application object.
    */
   void focusChanged(ViewerPtr viewer);

   event::ResultType objectsSelected(const std::vector<SceneObjectPtr>& objs,
		                     bool selected);
   event::ResultType objectsMovedSlot(const EventData::moved_obj_list_t&);

private:
   static std::string getElementType()
   {
      return std::string("vrkit_widget_plugin");
   }

   /**
    * Responds to object intersection signals. If \p obj is newly intersected,
    * then our intersection highlight is applied to it.
    *
    * @post Our intersection highlight is applied to \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    * @param pnt         The intersection point.
    */
   event::ResultType objectIntersected(SceneObjectPtr obj,
                                       const gmtl::Point3f& pnt);

   /**
    * Responds to object de-intersection signals. Our intersection highlight
    * is removed from \p obj.
    *
    * @post Our intersection highlight is removed from \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    */
   event::ResultType objectDeintersected(SceneObjectPtr obj);

   /**
    * Configures this plug-in.
    *
    * @pre The type of \p elt matches the value returned by getElementType().
    *
    * @param elt The config element for this plug-in.
    *
    * @throw vrkit::Exception
    *           Thrown if the given config element contains bad data, is out
    *           of date, or is otherwise unusable.
    */
   void configure(jccl::ConfigElementPtr elt);

   WandInterfacePtr mWandInterface;

   /** @name Digital Buttons */
   //@{
   util::DigitalCommand mSelectBtn;   /**< Activate/deactivate all button */
   util::DigitalCommand mSelect1Btn;  /**< Cycle grid selection button */
   //@}

   /** @name Digital Button Command Descriptions */
   //@{
   const std::string mSelectText;
   const std::string mSelect1Text;
   //@}

   bool mIntersecting;
   bool mWidgetPressed;
   SceneObjectPtr mIntersectedObj;

   /** @name Intersection Strategy */
   //@{
   gmtl::Point3f mIntersectPoint;
   //@}

   /** @name Intersection Strategy */
   //@{
   boost::signals::connection mIsectConnection;
   boost::signals::connection mDeIsectConnection;
   //@}

   WidgetDataPtr mWidgetData;
   boost::signals::connection mMovedConnection;
   SceneObjectPtr mSelectedObject;
};

}


#endif /* _VRKIT_GRID_PLUGIN_H_ */
