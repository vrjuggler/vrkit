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

#ifndef _VRKIT_WIDGET_MATERIAL_CHOOSER_H_
#define _VRKIT_WIDGET_MATERIAL_CHOOSER_H_

#include <vrkit/Config.h>

#include <vector>
#include <boost/signal.hpp>

#include <OpenSG/OSGConfig.h>

#if OSG_MAJOR_VERSION < 2
#  include <OpenSG/OSGMaterialPool.h>
#else
#  include <OpenSG/OSGContainerPool.h>
#endif

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

   /**
    * @name Material Pool Types
    *
    * OpenSG 1.8/2.0 compatibility typedefs. These are mainly for internal
    * use, but they can be used in user-level code to help bridge the
    * differences between OpenSG 1.8 and 2.0.
    *
    * @see setMaterialPool()
    *
    * @since 0.51.0
    */
   //@{
#if OSG_MAJOR_VERSION < 2
   typedef OSG::MaterialPool     pool_t;
   typedef OSG::MaterialPoolPtr  pool_ptr_t;
#else
   typedef OSG::ContainerPool    pool_t;
   typedef OSG::ContainerPoolPtr pool_ptr_t;
#endif
   //@}

   void setMaterialPool(pool_ptr_t matPool);

   // SLOTS
   void materialsChanged();

   // SIGNALS
   boost::signal<int (OSG::MaterialPtr), event::ResultOperator>
      mMaterialSelectedSignal;

   event::ResultType onButtonClicked(unsigned int index);
   event::ResultType onScrolled(int value);

protected:
#if OSG_MAJOR_VERSION >= 2
   /**
    * Extracts only field containers of type OSG::MaterialPtr (or a subclass
    * thereof) from \c mMaterialPool.
    *
    * @since 0.51.0
    */
   std::vector<OSG::MaterialPtr> getMaterials();
#endif

   pool_ptr_t                   mMaterialPool;
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
