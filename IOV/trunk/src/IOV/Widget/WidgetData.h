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

#ifndef _INF_WIDGET_DATA_H_
#define _INF_WIDGET_DATA_H_

#include <IOV/Config.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/Widget/WidgetDataPtr.h>
#include <IOV/Widget/WidgetPtr.h>

#include <vector>

namespace inf
{

class IOV_CLASS_API WidgetData : public inf::SceneData
{
public:
   typedef std::vector<SceneObjectPtr> widget_list_t;

   /**
    * The unique type identifier for scene data used by plug-ins that manage
    * widgets in the scene.
    */
   static const vpr::GUID type_guid;

   static WidgetDataPtr create()
   {
      return WidgetDataPtr(new WidgetData);
   }

   virtual ~WidgetData();

   void addWidget(SceneObjectPtr widget);

   /**
    * Remove given widget from widget list.
    */
   void removeWidget(SceneObjectPtr widget);

   const widget_list_t& getWidgets() const
   {
      return mWidgets;
   }

   bool widgetsAddedOrRemoved()
   {
      return mWidgetsAddedOrRemoved;
   }

   void setWidgetsAddedOrRemoved(bool val = true)
   {
      mWidgetsAddedOrRemoved = val;
   }

protected:
   WidgetData();

   widget_list_t mWidgets;
   bool mWidgetsAddedOrRemoved;
};

}


#endif
