// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_MATERIAL_CHOOSER_H_
#define _INF_MATERIAL_CHOOSER_H_

#include <IOV/Config.h>

#include <IOV/Event.h>
#include <IOV/Widget/Frame.h>
#include <IOV/Widget/SphereButtonPtr.h>
#include <IOV/Widget/ScrollBarPtr.h>
#include <IOV/Widget/MaterialChooserPtr.h>

#include <OpenSG/OSGMaterialPool.h>

#include <boost/signal.hpp>
#include <vector>

namespace inf
{

class IOV_CLASS_API MaterialChooser : public Frame
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
    * @return This object is returned as a shared pointer. A inf::WidgetPtr is
    *         returned rather than inf::FramePtr because these types do not
    *         allow for covariant return types in the method override.
    */
   virtual WidgetPtr init(const float metersToAppUnits);

public:  // Configuration params //
   virtual void setWidthHeight(const float w, const float h, const float borderWidth=0.0f);
   void setMaterialPool(OSG::MaterialPoolPtr matPool);

   // SLOTS
   void materialsChanged();

   // SIGNALS
   boost::signal< int (OSG::MaterialPtr), Event::ResultOperator > mMaterialSelectedSignal;

   Event::ResultType onButtonClicked(unsigned int index);
   Event::ResultType onScrolled(int value);
private:

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

#endif

