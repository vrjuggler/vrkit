#include <IOV/StatusPanelViewOriginal.h>

#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGClipPlaneChunk.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <vpr/Util/Assert.h>

#include <sstream>

#include <boost/bind.hpp>

namespace inf
{

StatusPanelViewOriginal::StatusPanelViewOriginal()
{
   mIsDirty = true;
   mFont = NULL;

   mTitleHeight = 0.08f;
   mStatusHeight = 0.375f;
   mHeaderHeight = 0.175f;
   mStatusTextHeight = 0.5f;

   mBgColor.setValuesRGB(0.2, 0.2, 0.2);
   mBgAlpha = 0.6f;
   mBorderColor.setValuesRGB(1, 1, 1);
   mTitleColor.setValuesRGB(1, 0.5, 0);
   mTextColor.setValuesRGB(1, 1, 1);
   
   mDrawDebug = false;
}

void StatusPanelViewOriginal::initialize(const float metersToAppUnits, StatusPanel* const panel)
{
   mStatusPanel = panel;
   
   mStatusPanel->statusPanelChanged().connect(
         boost::bind(&StatusPanelViewOriginal::setDirty, this));
   
   mMetersToAppUnits = metersToAppUnits;
   
   const float feet_to_app_units(0.3048f * mMetersToAppUnits);

   mPanWidth = 10.0f * feet_to_app_units;
   mPanHeight = 15.0f * feet_to_app_units;
   mBorderWidth = 0.4f * feet_to_app_units;
   mBorderDepth = 0.2f * feet_to_app_units;
   
   mRootPanelNode = OSG::Node::create();
   mPanelGeomNode = OSG::Node::create();
   mPanelGeomCore = mBuilder.createGeomGeo();

   mTextGeomNode = OSG::Node::create();
   mTextGeomCore = mBuilder.createTextGeom();

   OSG::setName(mRootPanelNode, "RootPanelNode");
   OSG::setName(mPanelGeomNode, "PanelGeomNode");
   OSG::setName(mTextGeomNode, "TextGeomNode");
   OSG::setName(mPanelGeomCore, "PanelGeomCore");
   OSG::setName(mTextGeomCore, "TextGeomCore");

   OSG::CPEditor rpne(mRootPanelNode);
   OSG::CPEditor pgne(mPanelGeomNode);
   OSG::CPEditor pgce(mPanelGeomCore);
   OSG::CPEditor tgne(mTextGeomNode);
   OSG::CPEditor tgce(mTextGeomCore);

   mPanelGeomNode->setCore(mPanelGeomCore);
   mTextGeomNode->setCore(mTextGeomCore);

   mRootPanelNode->setCore(OSG::Group::create());
   mRootPanelNode->addChild(mPanelGeomNode);
   mRootPanelNode->addChild(mTextGeomNode);

   mFont = new inf::UiBuilder::Font("SANS", OSG::TextFace::STYLE_PLAIN, 64);
   //mFont = new inf::UiBuilder::Font("MONO", OSG::TextFace::STYLE_PLAIN, 64);

   OSG::ChunkMaterialPtr text_mat = OSG::ChunkMaterialPtr::dcast(mTextGeomCore->getMaterial());
   vprASSERT(OSG::NullFC != text_mat);

   mClipRight = OSG::ClipPlaneChunk::create();
   beginEditCP(mClipRight);
   mClipRight->setEquation(OSG::Vec4f(-1,0,0,mPanWidth));      // X clip plane <= right size
   mClipRight->setEnable(true);
   mClipRight->setBeacon(mTextGeomNode);
   endEditCP(mClipRight);

   mClipBottom = OSG::ClipPlaneChunk::create();
   beginEditCP(mClipBottom);
   mClipBottom->setEquation(OSG::Vec4f(0,1,0,0));         // Y clip plane on Y>=0
   mClipBottom->setEnable(true);
   mClipBottom->setBeacon(mTextGeomNode);
   endEditCP(mClipBottom);

   OSG::beginEditCP(text_mat);
   text_mat->addChunk(mClipBottom);
   text_mat->addChunk(mClipRight);
   OSG::endEditCP(text_mat);
   setDirty();
}

void StatusPanelViewOriginal::setDirty()
{
   mIsDirty = true;
}


void StatusPanelViewOriginal::update()
{
   if(mIsDirty)
   {
      updatePanelScene();
      mIsDirty = false;
   }
}

void StatusPanelViewOriginal::setWidthHeight(const float w, const float h, const float borderWidth)
{
   mPanWidth = w;
   mPanHeight = h;

   mBorderWidth = borderWidth;
   if(0.0f == borderWidth)
   {
      mBorderWidth = ((mPanWidth+mPanHeight)/2.0f)/20.0f;
   }
   mBorderDepth = mBorderWidth/2.0f;

   setDirty();
}

void StatusPanelViewOriginal::updatePanelScene()
{
   mBuilder.resetGeomGeo(mPanelGeomCore);

   // Draw the panel outline
   OSG::Vec2f panel_ll(0,0), panel_ul(0,mPanHeight), panel_ur(mPanWidth,mPanHeight), panel_rl(mPanWidth,0);

   const float feet_to_app_units(0.3048f * mMetersToAppUnits);

   const float inner_rad(0.2f * feet_to_app_units);
   unsigned int num_segs(8);
   const float front_depth( mBorderDepth/2.0f);
   const float back_depth (-mBorderDepth/2.0f);
   //const float bg_depth(-0.1f * feet_to_app_units);

   mBuilder.buildRoundedRectangle(mPanelGeomCore, mBorderColor, panel_ll, panel_ur, inner_rad, inner_rad+mBorderWidth,
                                  num_segs, false, front_depth, back_depth, 1.0);
   mBuilder.buildRoundedRectangle(mPanelGeomCore, mBgColor,     panel_ll, panel_ur, 0.0, inner_rad+(mBorderWidth*1.5f),
                                  num_segs, true,  back_depth,    back_depth, mBgAlpha);

   const float text_spacing(0.7);
   float abs_title_height = mTitleHeight * mPanHeight;
   const float title_indent(0.1 * mPanWidth);

   OSG::Color3f dbg_color(1, 0, 0);

   // ---- Header --- //
   // Title
   float total_header_height = mHeaderHeight * mPanHeight;
   float total_status_height = mStatusHeight * mPanHeight;
   float total_center_height = (1.0f - mHeaderHeight - mStatusHeight) * mPanHeight;
   float header_pan_height = total_header_height-abs_title_height;
   float center_pan_height = total_center_height-abs_title_height;
   float status_pan_height = total_status_height-abs_title_height;

   OSG::Vec2f header_title_ul(title_indent, mPanHeight);
   OSG::Vec2f header_ul(0, header_title_ul.y()-abs_title_height);
   OSG::Vec2f center_title_ul(title_indent, header_title_ul.y()-total_header_height);
   OSG::Vec2f center_ul(0, center_title_ul.y()-abs_title_height);
   OSG::Vec2f status_title_ul(title_indent, center_title_ul.y()-total_center_height);
   OSG::Vec2f status_ul(0, status_title_ul.y()-abs_title_height);

   // Headers
   mBuilder.buildText(mTextGeomCore, *mFont, mStatusPanel->getHeaderTitle(), header_title_ul, mTitleColor, abs_title_height, text_spacing);
   mBuilder.addText(mTextGeomCore, *mFont, mStatusPanel->getCenterTitle(), center_title_ul, mTitleColor, abs_title_height, text_spacing);
   mBuilder.addText(mTextGeomCore, *mFont, mStatusPanel->getBottomTitle(), status_title_ul, mTitleColor, abs_title_height, text_spacing);

   // Header section
   OSG::Vec2f bounds = mBuilder.getTextSize(*mFont, mStatusPanel->getHeaderText(), text_spacing);
   float pan_scale = OSG::osgMin( (header_pan_height/bounds.y()), (mPanWidth/bounds.x()));
   mBuilder.addText(mTextGeomCore, *mFont, mStatusPanel->getHeaderText(), header_ul, mTextColor, pan_scale, text_spacing);

   // Center section
   std::stringstream center_text_stream;
   center_text_t center_text = mStatusPanel->getCenterText();
   center_text_t::iterator i;
   for ( i = center_text.begin(); i != center_text.end(); ++i )
   {
      if ( ! (*i).first.empty() && ! (*i).second.empty() )
      {
         std::stringstream key_stream;
         for ( unsigned int j = 0; j < (*i).first.size(); ++j )
         {
            key_stream << (*i).first[j];
            if ( j + 1 < (*i).first.size() )
            {
               key_stream << "+";
            }
         }

         std::stringstream cmd_stream;
         for ( unsigned int j = 0; j < (*i).second.size(); ++j )
         {
            cmd_stream << (*i).second[j];
            if ( j + 1 < (*i).second.size() )
            {
               cmd_stream << "; ";
            }
         }

         center_text_stream << key_stream.str() << ": " << cmd_stream.str()
               << std::endl;
      }
   }

   bounds = mBuilder.getTextSize(*mFont, center_text_stream.str(),
                                  text_spacing);
   pan_scale = OSG::osgMin( (center_pan_height / bounds.y()), (mPanWidth / bounds.x()));
   mBuilder.addText(mTextGeomCore, *mFont, center_text_stream.str(),
                    center_ul, mTextColor, pan_scale, text_spacing);

   // Status panel
   unsigned int num_lines( (unsigned int)(status_pan_height / mStatusTextHeight) );
   if ( num_lines > mStatusPanel->getStatusLines().size() )
   {
      num_lines = mStatusPanel->getStatusLines().size();
   }

   std::deque<std::string> status_lines = mStatusPanel->getStatusLines();
   std::vector<std::string> potential_lines;
   potential_lines.insert(potential_lines.end(), status_lines.begin(),
                          status_lines.begin() + num_lines);

   bounds = mBuilder.getTextSize(*mFont, potential_lines, text_spacing);
   float status_pan_scale = OSG::osgMin(status_pan_height / bounds.y(),
                                        mPanWidth / bounds.x());

   mBuilder.addText(mTextGeomCore, *mFont, potential_lines, status_ul,
                    mTextColor, status_pan_scale, text_spacing);

   // -- Update the materials and other properties --- //
   beginEditCP(mClipRight);
   mClipRight->setEquation(OSG::Vec4f(-1,0,0,mPanWidth));      // X clip plane <= right size
   endEditCP(mClipRight);

   beginEditCP(mClipBottom);
   mClipBottom->setEquation(OSG::Vec4f(0,1,0,0));         // Y clip plane on Y>=0
   endEditCP(mClipBottom);

   // --- Draw debug outlines --- //
   if(mDrawDebug)
   {
      mBuilder.buildRectangleOutline(mPanelGeomCore, dbg_color, panel_ll, panel_ur, 0.2);
      mBuilder.buildRectangleOutline(mPanelGeomCore, OSG::Color3f(1,1,0),
                                     OSG::Vec2f(0,header_title_ul.y()-total_header_height),
                                     OSG::Vec2f(mPanWidth, header_title_ul.y()), 0.3);       // Header
      mBuilder.buildRectangleOutline(mPanelGeomCore, OSG::Color3f(0,1,0),
                                     OSG::Vec2f(0,center_title_ul.y()-total_center_height),
                                     OSG::Vec2f(mPanWidth, center_title_ul.y()), 0.4);       // Center
      mBuilder.buildRectangleOutline(mPanelGeomCore, OSG::Color3f(0,0,1),
                                     OSG::Vec2f(0,status_title_ul.y()-total_status_height),
                                     OSG::Vec2f(mPanWidth, status_title_ul.y()), 0.3);       // Status
   }
}

}
