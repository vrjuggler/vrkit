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

#ifndef _VRKIT_WIDGET_MATERIAL_CHOOSER_H_
#define _VRKIT_WIDGET_MATERIAL_CHOOSER_H_

#include <vrkit/Config.h>

#include <vector>
#include <boost/signal.hpp>

#include <OpenSG/OSGMaterialPool.h>

#include <vrkit/scenedata/Event.h>
#include <vrkit/widget/Frame.h>
#include <vrkit/widget/SphereButtonPtr.h>
#include <vrkit/widget/ScrollBarPtr.h>
#include <vrkit/widget/MaterialChooserPtr.h>


namespace vrkit
{

namespace widget
{

/** \class MaterialChooser MaterialChooser.h vrkit/widget/MaterialChooser.h
 *
 * @note This class was moved into the vrkit::widget namespace in version 0.47.
 */
class VRKIT_CLASS_API MaterialChooser : public Frame
{
protected:
   MaterialChooser();

public:
   static MaterialChooserPtr create()
   {
      return MaterialChooserPtr(new MaterialChooser());
   }

   /**
    * Initializes scene graph, fonts, and everything else that is used.
    *
    * @param metersToAppUnits The conversion factor from meters (VR Juggler's
    *                         internal units) to the application-specific
    *                         units.
    *
    * @return This object is returned as a shared pointer. A vrkit::WidgetPtr
    *         is returned rather than vrkit::FramePtr because these types do
    *         not allow for covariant return types in the method override.
    */
   virtual WidgetPtr init(const float metersToAppUnits);

public:  // Configuration params //
   virtual void setWidthHeight(const float w, const float h,
                               const float borderWidth = 0.0f);
   void setMaterialPool(OSG::MaterialPoolPtr matPool);

   // SLOTS
   void materialsChanged();

   // SIGNALS
   boost::signal<int (OSG::MaterialPtr), event::ResultOperator>
      mMaterialSelectedSignal;

   event::ResultType onButtonClicked(unsigned int index);
   event::ResultType onScrolled(int value);

protected:
   OSG::MaterialPoolPtr         mMaterialPool;
   std::vector<SphereButtonPtr> mMaterialButtons;
   ScrollBarPtr                 mScrollBar;
   int   mHNum;
   int   mVNum;
   float mSpacing;
   float mVSpacing;
   float mHSpacing;
   float mSphereRadius;
};

}

}


#endif /* _VRKIT_WIDGET_MATERIAL_CHOOSER_H_ */
