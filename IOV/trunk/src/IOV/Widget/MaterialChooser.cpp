// Copyright (C) Infiscape Corporation 2005-2007

#include <OpenSG/OSGSimpleAttachments.h>

#include <IOV/Widget/MaterialChooser.h>
#include <IOV/Widget/SphereButton.h>
#include <IOV/Widget/ScrollBar.h>

#include <vpr/Util/Assert.h>
#include <boost/bind.hpp>


namespace inf
{

MaterialChooser::MaterialChooser()
   : Frame(), mHNum(3), mVNum(3), mSpacing(0.25f)
{;}

WidgetPtr MaterialChooser::init(const float metersToAppUnits)
{
   mMaterialPool = OSG::MaterialPool::create();

   WidgetPtr myself = Frame::init(metersToAppUnits);

   for (int i = 0 ; i < mVNum ; i++)
   {
      for (int j = 0 ; j < mHNum ; j++)
      {
         SphereButtonPtr sb = SphereButton::create();
         addChild(sb);
         sb->init(mMetersToAppUnits);
         OSG::beginEditCP(mRootWidgetNode.node());
            mRootWidgetNode.node()->addChild(sb->getRoot());
         OSG::endEditCP(mRootWidgetNode.node());
         mMaterialButtons.push_back(sb);

      }
   }

   for (unsigned int i = 0 ; i < mMaterialButtons.size() ; i++)
   {
      mMaterialButtons[i]->mClickedSignal.connect(boost::bind(&MaterialChooser::onButtonClicked, this, i));
   }

   // Add scrollbar
   mScrollBar = ScrollBar::create();
   addChild(mScrollBar);
   mScrollBar->init(mMetersToAppUnits);
   OSG::beginEditCP(mRootWidgetNode.node());
      mRootWidgetNode.node()->addChild(mScrollBar->getRoot());
   OSG::endEditCP(mRootWidgetNode.node());

   mScrollBar->mValueChangedSignal.connect(boost::bind(&MaterialChooser::onScrolled, this, _1));

   setDirty();

   return myself;
}

Event::ResultType MaterialChooser::onScrolled(int value)
{
   int current_row = value;
   int num_materials = mMaterialPool->getCount();
   for (unsigned int i = 0 ; i < mMaterialButtons.size() ; i++)
   {
      int mat_index = (current_row*3) + i;
      if (mat_index < num_materials)
      {
         std::cout << "Mat Index: " << mat_index << std::endl;
         mMaterialButtons[i]->setMaterial(mMaterialPool->get(mat_index));
      }
      else
      {
         std::cout << "Empty Mat Index: " << mat_index << std::endl;
         mMaterialButtons[i]->setMaterial();
      }
   }
   return Event::CONTINUE;
}

Event::ResultType MaterialChooser::onButtonClicked(unsigned int index)
{
   OSG::MaterialPtr selected_material;

   std::string mat_str("<NULL>");

   int current_row = mScrollBar->value();
   unsigned int mat_index = (current_row*3) + index;

   if (mat_index >= mMaterialPool->getCount())
   {
      selected_material = OSG::NullFC;
   }
   else
   {
      selected_material = mMaterialPool->get(mat_index);
      const char* mat_name = OSG::getName(selected_material);
      if (NULL != mat_name)
      {
         mat_str = mat_name;
      }
   }

   std::cout << "Selected material: " << mat_str << std::endl;
   mMaterialSelectedSignal(selected_material);
   return Event::CONTINUE;
}

void MaterialChooser::setMaterialPool(OSG::MaterialPoolPtr matPool)
{
   mMaterialPool = matPool;
   materialsChanged();
}

void MaterialChooser::materialsChanged()
{
   std::set<OSG::MaterialPtr> mats = mMaterialPool->get();
   std::set<OSG::MaterialPtr>::iterator m;

   for (m = mats.begin() ; m != mats.end() ; m++)
   {
      std::string mat_str("<NULL>");
      const char* mat_name = OSG::getName(*m);
      if (NULL != mat_name)
      {
         mat_str = mat_name;
      }
      std::cout << "Material: " << mat_str << std::endl;
   }

   OSG::UInt32 num_materials = mMaterialPool->getCount();
   int num_rows = num_materials/mHNum;

   if (num_materials % mHNum > 0)
   { num_rows++; }

   mScrollBar->setMinValue(0);
   mScrollBar->setMaxValue(num_rows);
   mScrollBar->setIncrement(1);
   onScrolled(mScrollBar->value());
}

void MaterialChooser::setWidthHeight(const float w, const float h, const float borderWidth)
{
   Frame::setWidthHeight(w, h, borderWidth);

   mSpacing = mWidth * 0.025;
   float hdiam = (mWidth - mSpacing*(mHNum+1))/mHNum;
   float vdiam = (mHeight - mSpacing*(mVNum+1))/mVNum;

   //float hdiam = (mWidth/mHNum);
   //float vdiam = (mHeight/mVNum);
   mSphereRadius = (hdiam < vdiam ? hdiam / 2 : vdiam / 2);

   mHSpacing = mWidth - (mSphereRadius * 2.0f * mHNum);
   mHSpacing /= mHNum+1;
   mVSpacing = mHeight - (mSphereRadius * 2.0f * mVNum);
   mVSpacing /= mVNum+1;

   std::cout << "PanWidth:  " << mWidth << std::endl;
   std::cout << "PanHeight: " << mHeight << std::endl;
   std::cout << "hdiam:     " << hdiam << std::endl;
   std::cout << "vdiam:     " << vdiam << std::endl;
   std::cout << "radius:    " << mSphereRadius << std::endl;
   std::cout << "mHSpacing: " << mHSpacing << std::endl;
   std::cout << "mVSpacing: " << mVSpacing << std::endl;

   for (int i = 0 ; i < mVNum ; i++)
   {
      for (int j = 0 ; j < mHNum ; j++)
      {
         SphereButtonPtr sb = mMaterialButtons[(i*mVNum)+j];

         float vpos = (mVSpacing * (i+1)) + (mSphereRadius * (1+(i*2)));
         float hpos = (mHSpacing * (j+1)) + (mSphereRadius * (1+(j*2)));
         sb->move(OSG::Pnt3f(hpos, vpos, 0.0f));
         sb->setWidthHeight(mSphereRadius, mSphereRadius, 0.0f);
         sb->update();
      }
   }
   mScrollBar->move(OSG::Pnt3f(mWidth, 0.0f, 0.0f));
   mScrollBar->setWidthHeight(mBorderWidth, mHeight, 0.0f);
   mScrollBar->update();
}


} // namespace inf

