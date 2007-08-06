// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_WIDGET_PLUGIN_H_
#define _INF_WIDGET_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/EventData.h>
#include <IOV/Plugin.h>
#include <IOV/ViewerPtr.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/Grab/IntersectionStrategy.h>
#include <IOV/Util/DigitalCommand.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Widget/WidgetDataPtr.h>
#include <IOV/Widget/WidgetPtr.h>

#include <IOV/Event.h>
#include <gmtl/Matrix.h>

namespace inf
{

class WidgetPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<WidgetPlugin>
{
protected:
   WidgetPlugin(const inf::plugin::Info& info);

public:
   static inf::PluginPtr create(const inf::plugin::Info& info)
   {
      return inf::PluginPtr(new WidgetPlugin(info));
   }

   virtual ~WidgetPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("WidgetPlugin");
   }

   virtual PluginPtr init(inf::ViewerPtr viewer);

   /**
    * Updates the state of the widgets based on input from the wand interface.
    *
    * @post The state of all widgets is updated to reflect the digtial and/or
    *       analog input from the user via the wand interface.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(inf::ViewerPtr viewer);

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
   void focusChanged(inf::ViewerPtr viewer);

   inf::Event::ResultType
      objectsSelected(const std::vector<inf::SceneObjectPtr>& objs,
		      bool selected);
   inf::Event::ResultType objectsMovedSlot(const EventData::moved_obj_list_t&);

private:
   static std::string getElementType()
   {
      return std::string("iov_widget_plugin");
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
   inf::Event::ResultType objectIntersected(inf::SceneObjectPtr obj,
                                            const gmtl::Point3f& pnt);

   /**
    * Responds to object de-intersection signals. Our intersection highlight
    * is removed from \p obj.
    *
    * @post Our intersection highlight is removed from \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    */
   inf::Event::ResultType objectDeintersected(inf::SceneObjectPtr obj);

   /**
    * Configures this plug-in.
    *
    * @pre The type of \p elt matches the value returned by getElementType().
    *
    * @param elt The config element for this plug-in.
    *
    * @throw inf::Exception is thrown if the given config element contains
    *        bad data, is out of date, or is otherwise unusable.
    */
   void configure(jccl::ConfigElementPtr elt);

   inf::WandInterfacePtr mWandInterface;

   /** @name Digital Buttons */
   //@{
   inf::DigitalCommand mSelectBtn;    /**< Activate/deactivate all button */
   inf::DigitalCommand mSelect1Btn;       /**< Cycle grid selection button */
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


#endif /* _INF_GRID_PLUGIN_H_ */
