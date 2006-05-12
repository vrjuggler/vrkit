// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WIDGET_PLUGIN_H_
#define _INF_WIDGET_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/Plugin.h>
#include <IOV/PluginFactory.h>
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
public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new WidgetPlugin());
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
   virtual void updateState(inf::ViewerPtr viewer);

   /**
    * Does nothing.
    *
    * @param viewer The VR Juggler application object. This parameter is
    *               ignored.
    */
   virtual void run(inf::ViewerPtr viewer);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

   WidgetPlugin();

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

   inf::Event::ResultType objectSelected(inf::SceneObjectPtr obj, bool selected);
   inf::Event::ResultType objectMovedSlot(inf::SceneObjectPtr obj, const gmtl::Matrix44f& newObjMat);

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
                                            inf::SceneObjectPtr parentObj,
                                            gmtl::Point3f pnt);

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

   /**
    * Transforms the given vector of zero-based button IDs to one-based IDs.
    *
    * @param btns A vector of zero-based button IDs to be transformed to use
    *             one-based IDs.
    *
    * @return A vector of one-based integers based on \p btns is returned.
    */
   std::vector<int> transformButtonVec(const std::vector<int>& btns);

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

   //std::vector<std::string> mStrategyPluginPath;
   inf::PluginFactoryPtr mPluginFactory;

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
