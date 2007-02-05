// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_WIDGET_MATERIAL_CHOOSER_PTR_H_
#define _INF_WIDGET_MATERIAL_CHOOSER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class MaterialChooser;
   typedef boost::shared_ptr<MaterialChooser> MaterialChooserPtr;
   typedef boost::weak_ptr<MaterialChooser> MaterialChooserWeakPtr;
}

#endif
