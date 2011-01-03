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

#include <boost/bind.hpp>

#include <OpenSG/OSGSimpleAttachments.h>

#include <vpr/Util/Assert.h>

#include <vrkit/widget/MaterialChooser.h>
#include <vrkit/widget/SphereButton.h>
#include <vrkit/widget/ScrollBar.h>


namespace vrkit
{

namespace widget
{

MaterialChooser::MaterialChooser()
   : Frame()
   , mHNum(3)
   , mVNum(3)
   , mSpacing(0.25f)
{
   /* Do nothing. */ ;
}

WidgetPtr MaterialChooser::init(const float metersToAppUnits)
{
   mMaterialPool = pool_t::create();

   WidgetPtr myself = Frame::init(metersToAppUnits);

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rwne(mRootWidgetNode.node(), OSG::Node::ChildrenFieldMask);
#endif

   for ( int i = 0; i < mVNum; ++i )
   {
      for ( int j = 0; j < mHNum; ++j )
      {
         SphereButtonPtr sb = SphereButton::create();
         addChild(sb);
         sb->init(mMetersToAppUnits);
         mRootWidgetNode.node()->addChild(sb->getRoot());
         mMaterialButtons.push_back(sb);
      }
   }

   for ( unsigned int i = 0; i < mMaterialButtons.size(); ++i )
   {
      mMaterialButtons[i]->mClickedSignal.connect(
         boost::bind(&MaterialChooser::onButtonClicked, this, i)
      );
   }

   // Add scrollbar
   mScrollBar = ScrollBar::create();
   addChild(mScrollBar);
   mScrollBar->init(mMetersToAppUnits);
   mRootWidgetNode.node()->addChild(mScrollBar->getRoot());

   mScrollBar->mValueChangedSignal.connect(
      boost::bind(&MaterialChooser::onScrolled, this, _1)
   );

   setDirty();

   return myself;
}

event::ResultType MaterialChooser::onScrolled(int value)
{
   int current_row = value;
#if OSG_MAJOR_VERSION >= 2
   std::vector<OSG::MaterialPtr> mats = getMaterials();
#endif
   const OSG::UInt32 num_materials =
#if OSG_MAJOR_VERSION < 2
      mMaterialPool->getCount();
#else
      mats.size();
#endif

   for ( unsigned int i = 0; i < mMaterialButtons.size(); ++i )
   {
      const unsigned int mat_index = current_row * 3 + i;
      if ( mat_index < num_materials )
      {
         std::cout << "Mat Index: " << mat_index << std::endl;
         mMaterialButtons[i]->setMaterial(
#if OSG_MAJOR_VERSION < 2
            mMaterialPool->get(mat_index)
#else
            mats[mat_index]
#endif
         );
      }
      else
      {
         std::cout << "Empty Mat Index: " << mat_index << std::endl;
         mMaterialButtons[i]->setMaterial();
      }
   }
   return event::CONTINUE;
}

event::ResultType MaterialChooser::onButtonClicked(unsigned int index)
{
   OSG::MaterialPtr selected_material;

   std::string mat_str("<NULL>");

   int current_row = mScrollBar->value();
   unsigned int mat_index = current_row * 3 + index;

   const OSG::UInt32 num_materials =
#if OSG_MAJOR_VERSION < 2
      mMaterialPool->getCount();
#else
      mMaterialPool->getNContainers();
#endif

   if ( mat_index >= num_materials )
   {
      selected_material = OSG::NullFC;
   }
   else
   {
      selected_material =
#if OSG_MAJOR_VERSION < 2
         mMaterialPool->get(mat_index);
#else
         getMaterials()[mat_index];
#endif
      const char* mat_name = OSG::getName(selected_material);
      if ( NULL != mat_name )
      {
         mat_str = mat_name;
      }
   }

   std::cout << "Selected material: " << mat_str << std::endl;
   mMaterialSelectedSignal(selected_material);
   return event::CONTINUE;
}

void MaterialChooser::setMaterialPool(pool_ptr_t matPool)
{
   mMaterialPool = matPool;
   materialsChanged();
}

void MaterialChooser::materialsChanged()
{
#if OSG_MAJOR_VERSION < 2
   const std::set<OSG::MaterialPtr>& mats = mMaterialPool->get();
   typedef std::set<OSG::MaterialPtr>::const_iterator iter_type;
#else
   std::vector<OSG::MaterialPtr> mats = getMaterials();
   typedef std::vector<OSG::MaterialPtr>::iterator iter_type;
#endif

   for ( iter_type m = mats.begin(); m != mats.end(); ++m )
   {
      std::string mat_str("<NULL>");

#if OSG_MAJOR_VERSION < 2
      const char* mat_name = OSG::getName(*m);
#else
      const char* mat_name =
         OSG::getName(
            OSG::cast_dynamic<OSG::AttachmentContainerPtr>(*m)
         );
#endif

      if (NULL != mat_name)
      {
         mat_str = mat_name;
      }
      std::cout << "Material: " << mat_str << std::endl;
   }

   const OSG::UInt32 num_materials =
#if OSG_MAJOR_VERSION < 2
      mMaterialPool->getCount();
#else
      mats.size();
#endif
   int num_rows = num_materials/mHNum;

   if ( num_materials % mHNum > 0 )
   {
      ++num_rows;
   }

   mScrollBar->setMinValue(0);
   mScrollBar->setMaxValue(num_rows);
   mScrollBar->setIncrement(1);
   onScrolled(mScrollBar->value());
}

void MaterialChooser::setWidthHeight(const float w, const float h,
                                     const float borderWidth)
{
   Frame::setWidthHeight(w, h, borderWidth);

   mSpacing = mWidth * 0.025;
   float hdiam = (mWidth - mSpacing * (mHNum + 1)) / mHNum;
   float vdiam = (mHeight - mSpacing * (mVNum +1)) / mVNum;

   //float hdiam = (mWidth/mHNum);
   //float vdiam = (mHeight/mVNum);
   mSphereRadius = (hdiam < vdiam ? hdiam / 2 : vdiam / 2);

   mHSpacing = mWidth - (mSphereRadius * 2.0f * mHNum);
   mHSpacing /= mHNum + 1;
   mVSpacing = mHeight - (mSphereRadius * 2.0f * mVNum);
   mVSpacing /= mVNum + 1;

   std::cout << "PanWidth:  " << mWidth << std::endl;
   std::cout << "PanHeight: " << mHeight << std::endl;
   std::cout << "hdiam:     " << hdiam << std::endl;
   std::cout << "vdiam:     " << vdiam << std::endl;
   std::cout << "radius:    " << mSphereRadius << std::endl;
   std::cout << "mHSpacing: " << mHSpacing << std::endl;
   std::cout << "mVSpacing: " << mVSpacing << std::endl;

   for ( int i = 0; i < mVNum; ++i )
   {
      for ( int j = 0; j < mHNum; ++j )
      {
         SphereButtonPtr sb = mMaterialButtons[i * mVNum + j];

         float vpos = mVSpacing * (i + 1) + mSphereRadius * (1 + i * 2);
         float hpos = mHSpacing * (j + 1) + mSphereRadius * (1 + j * 2);
         sb->move(OSG::Pnt3f(hpos, vpos, 0.0f));
         sb->setWidthHeight(mSphereRadius, mSphereRadius, 0.0f);
         sb->update();
      }
   }
   mScrollBar->move(OSG::Pnt3f(mWidth, 0.0f, 0.0f));
   mScrollBar->setWidthHeight(mBorderWidth, mHeight, 0.0f);
   mScrollBar->update();
}

#if OSG_MAJOR_VERSION >= 2
std::vector<OSG::MaterialPtr> MaterialChooser::getMaterials()
{
   std::vector<OSG::MaterialPtr> materials;

   const OSG::MFFieldContainerPtr& containers = mMaterialPool->getContainers();
   typedef OSG::MFFieldContainerPtr::const_iterator iter_type;

   // Count the containers that are derived from OSG::Material.
   for ( iter_type c = containers.begin(); c != containers.end(); ++c )
   {
      if ( (*c)->getType().isDerivedFrom(OSG::Material::getClassType()) )
      {
         materials.push_back(OSG::cast_dynamic<OSG::MaterialPtr>(*c));
      }
   }

   return materials;
}
#endif

} // namespace widget

} // namespace vrkit
