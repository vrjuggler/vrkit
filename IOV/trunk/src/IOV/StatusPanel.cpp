#include <IOV/StatusPanel.h>

#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGClipPlaneChunk.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <algorithm>

StatusPanel::StatusPanel()
{
   mFont = NULL;

   mPanWidth = 10.0f;
   mPanHeight = 15.0f;
   mBorderWidth = 0.4f;

   mTitleHeight = 0.05f;
   mStatusHeight = 0.30f;
   mHeaderHeight = 0.20f;
   mStatusHistorySize = 20;
   mStatusTextHeight = 0.5f;

   mBgColor.setValuesRGB(0.5, 0.5, 0.5);
   mBgAlpha = 1.0f;
   mBorderColor.setValuesRGB(1,1,1);
   mTitleColor.setValuesRGB(1,0.5,0);
   mTextColor.setValuesRGB(1,1,1);

   mHeaderTitle = "Header";
   mCenterTitle = "Center";
   mBottomTitle = "Status";

   mHeaderText  = "Header\nText";
   mCenterText  = "Controls:\n1 - Forward\n2 - Rotate\n3 - Viewport Cycle\n4 - Switch Mode\nReally long line of text here to test it.";
}


void StatusPanel::initialize()
{

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

   OSG::ClipPlaneChunkPtr clip_right = OSG::ClipPlaneChunk::create();
   beginEditCP(clip_right);
   clip_right->setEquation(OSG::Vec4f(-1,0,0,mPanWidth));      // X clip plane <= right size
   clip_right->setEnable(true);
   clip_right->setBeacon(mTextGeomNode);
   endEditCP(clip_right);

   OSG::ClipPlaneChunkPtr clip_bottom = OSG::ClipPlaneChunk::create();
   beginEditCP(clip_bottom);
   clip_bottom->setEquation(OSG::Vec4f(0,1,0,0));         // Y clip plane on Y>=0
   clip_bottom->setEnable(true);
   clip_bottom->setBeacon(mTextGeomNode);
   endEditCP(clip_bottom);

   OSG::beginEditCP(text_mat);
   text_mat->addChunk(clip_right);
   text_mat->addChunk(clip_bottom);
   OSG::endEditCP(text_mat);

}

void StatusPanel::setHeaderTitle(std::string txt)
{
   mHeaderTitle = txt;
   updatePanelScene();
}

void StatusPanel::setCenterTitle(std::string txt)
{
   mCenterTitle = txt;
   updatePanelScene();
}

void StatusPanel::setBottomTitle(std::string txt)
{
   mBottomTitle = txt;
   updatePanelScene();
}


void StatusPanel::setHeaderText(std::string header)
{
   mHeaderText = header;
   updatePanelScene();
}

void StatusPanel::setControlText(std::string text)
{
   mCenterText = text;
   updatePanelScene();
}

void StatusPanel::addStatusMessage(std::string msg)
{
   mStatusLines.push_front(msg);

   // Shrink status lines to fit.
   while(mStatusLines.size() > mStatusHistorySize)
   {
      mStatusLines.pop_back();
   }
   updatePanelScene();
}

void StatusPanel::updatePanelScene()
{
   // Draw the panel outline
   OSG::Vec2f panel_ll(0,0), panel_ul(0,mPanHeight), panel_ur(mPanWidth,mPanHeight), panel_rl(mPanWidth,0);

   const float inner_rad(0.2f);
   unsigned    num_segs(8);
   const float front_depth(0.1f), back_depth(-0.1f);
   //const float bg_depth(-0.1f);

   mBuilder.buildRoundedRectangle(mPanelGeomCore, mBorderColor, panel_ll, panel_ur, inner_rad, inner_rad+mBorderWidth,
                                 num_segs, false, front_depth, back_depth, 1.0);
   //mBuilder.buildRoundedRectangle(mPanelGeomCore, mBgColor,     panel_ll, panel_ur, 0.0, inner_rad+(mBorderWidth*2),
   //                              num_segs, true,  back_depth,    back_depth, mBgAlpha);

   const float text_spacing(0.7);
   float abs_title_height = mTitleHeight*mPanHeight;
   const float title_indent(0.1 * mPanWidth);

   OSG::Color3f dbg_color(1,0,0);

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
   mBuilder.buildText(mTextGeomCore, *mFont, mHeaderTitle, header_title_ul, mTitleColor, abs_title_height, text_spacing);
   mBuilder.addText(mTextGeomCore, *mFont, mCenterTitle, center_title_ul, mTitleColor, abs_title_height, text_spacing);
   mBuilder.addText(mTextGeomCore, *mFont, mBottomTitle, status_title_ul, mTitleColor, abs_title_height, text_spacing);

   // Header section
   OSG::Vec2f bounds = mBuilder.getTextSize(*mFont, mHeaderText, text_spacing);
   float pan_scale = OSG::osgMin( (header_pan_height/bounds.y()), (mPanWidth/bounds.x()));
   mBuilder.addText(mTextGeomCore, *mFont, mHeaderText, header_ul, mTextColor, pan_scale, text_spacing);

   // Center section
   bounds = mBuilder.getTextSize(*mFont, mCenterText, text_spacing);
   pan_scale = OSG::osgMin( (center_pan_height/bounds.y()), (mPanWidth/bounds.x()));
   mBuilder.addText(mTextGeomCore, *mFont, mCenterText, center_ul, mTextColor, pan_scale, text_spacing);

   // Status panel
   unsigned num_lines(unsigned(status_pan_height/mStatusTextHeight));
   if(num_lines > mStatusLines.size())
   { num_lines = mStatusLines.size(); }

   std::vector<std::string> potential_lines;
   potential_lines.insert(potential_lines.end(), mStatusLines.begin(), mStatusLines.begin()+num_lines);
   mBuilder.addText(mTextGeomCore, *mFont, potential_lines, status_ul, mTextColor, mStatusTextHeight, text_spacing);


   // --- Draw debug outlines --- //
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
