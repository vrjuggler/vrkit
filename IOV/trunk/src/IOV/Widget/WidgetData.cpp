// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Widget/WidgetData.h>
#include <IOV/SceneObject.h>
#include <algorithm>

namespace inf
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
