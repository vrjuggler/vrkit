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

#include <algorithm>
#include <vrkit/scenedata/WidgetData.h>


namespace vrkit
{

const vpr::GUID WidgetData::type_guid("86a3a623-3155-446d-aa4e-0906e1477b3b");

WidgetData::~WidgetData()
{
   /* Do nothing. */ ;
}

WidgetData::WidgetData() : mWidgetsAddedOrRemoved(false)
{
   /* Do nothing. */ ;
}

void WidgetData::addWidget(SceneObjectPtr widget)
{
   mWidgets.push_back(widget);
   setWidgetsAddedOrRemoved();
}

void WidgetData::removeWidget(SceneObjectPtr widget)
{
   widget_list_t::iterator found
      = std::find(mWidgets.begin(), mWidgets.end(), widget);

   if (mWidgets.end() != found)
   {
      mWidgets.erase(found);
      setWidgetsAddedOrRemoved();
   }
}

}
