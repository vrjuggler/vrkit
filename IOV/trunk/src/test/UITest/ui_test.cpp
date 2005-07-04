// Test for UI code
//
// Just use simple scene manager and create a ui
// to display locally.
//

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGSimpleSceneManager.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <OpenSG/OSGTextTXFFace.h>
#include <OpenSG/OSGTextTXFGlyph.h>
#include <OpenSG/OSGTextFaceFactory.h>
#include <OpenSG/OSGTextLayoutParam.h>
#include <OpenSG/OSGTextLayoutResult.h>

#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGClipPlaneChunk.h>

#include <OpenSG/OSGPrimitiveIterator.h>

#include <OpenSG/OSGGeoFunctions.h>

#include <OpenSG/OSGVerifyGeoGraphOp.h>

#include <IOV/UiBuilder.h>
#include <vpr/vprConfig.h>
#include <vpr/Util/Assert.h>
#include <sstream>
#include <algorithm>

OSG::SimpleSceneManager* mgr;

std::stringstream    gOverlayTextStream;  /**< Text to write out each frame. */

int setupGLUT(int* argc, char* argv[]);

template<class OutIt>
void splitStr(
   const std::string& s,
   const std::string& sep,
   OutIt dest)
{
   std::string::size_type left = s.find_first_not_of( sep );
   std::string::size_type right = s.find_first_of( sep, left );
   while( left < right )
   {
      *dest = s.substr( left, right-left );
      ++dest;
      left = s.find_first_not_of( sep, right );
      right = s.find_first_of( sep, left );
   }
}


class VerifyTraverser
{
public:
   VerifyTraverser()
      : mVerbose(true), mRepair(false), mNumErrors(0)
   {;}

   void setVerbose(bool val)
   { mVerbose = val; }

   void setRepair(bool val)
   { mRepair = val; }

   std::vector<OSG::NodePtr> getCorruptedNodes()
   { return mCorruptedNodes; }

   std::vector<OSG::GeometryPtr> getCorruptedGeos()
   { return mCorruptedGeos; }

   /*!
    * \brief
    * \param
    * \return
    */
   OSG::UInt32 verify(OSG::NodePtr &node)
   {
       if(node == NullFC)
           return 0;

       mCorruptedNodes.clear();
       mCorruptedGeos.clear();
       mNumErrors = 0;

       if(mVerbose)
       {
          std::cout << "Starting Verifier..." << std::endl;
       }

       //traverse(node, OSG::osgTypedFunctionFunctor1CPtrRef<OSG::Action::ResultE, OSG::NodePtr>(verifyCB));
       traverse(node,
                  OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<Action::ResultE,
                                                 VerifyTraverser,
                                                 NodePtr>(
                                                     this,
                                                     &VerifyTraverser::verifyCB));


       if(mRepair)
       {
          repairGeometry();      // Repair any corrupted geometry nodes
       }

       if(mVerbose)
       {
          std::cout << "Verifier completed. Errors found:" << mNumErrors << std::endl;
       }

       return mNumErrors;
   }

   /** Callback for each node. */
   OSG::Action::ResultE verifyCB(OSG::NodePtr &node)
   {
      if(OSG::NullFC == node)
      {
         std::cout << "verify CB called with NULL node, skipping." << std::endl;
         return OSG::Action::Continue;
      }

      // Verify basic node structure
      std::string node_name;
      if(OSG::getName(node) != NULL)
      { node_name = std::string(OSG::getName(node)); }

      OSG::NodeCorePtr node_core = node->getCore();
      if(OSG::NullFC == node_core)
      {
         mCorruptedNodes.push_back(node);
         mNumErrors += 1;
         if(mVerbose)
         { std::cout << "Node: [" << node_name << "] has NULL core." << std::endl; }
         if(mRepair)
         {
            if(mVerbose) std::cout << "  Repairing node.  Adding group core." << std::endl;
            OSG::beginEditCP(node);
            node->setCore(OSG::Group::create());
            OSG::endEditCP(node);
            std::string new_name = node_name + "_FIXED";
            OSG::setName(node, new_name);
         }
      }

      // Check based on core types
      if(OSG::NullFC != node_core)
      {
         if(GeometryPtr::dcast(node_core) != OSG::NullFC)
         {
            return verifyGeometryCB(node);
         }
      }

      return OSG::Action::Continue;

   }

protected:  // -- Geom --- //



   /** Verify geometry method. */
   OSG::Action::ResultE verifyGeometryCB(NodePtr &node)
   {
       GeometryPtr geo = GeometryPtr::dcast(node->getCore());

       if(geo == NullFC)
           return OSG::Action::Continue;

       if(geo->getPositions() == NullFC)
           return OSG::Action::Continue;

       OSG::UInt32 start_errors = mNumErrors;

       OSG::Int32 positions_size = geo->getPositions()->getSize();

       OSG::Int32 normals_size = 0;
       if(geo->getNormals() != NullFC)
           normals_size = geo->getNormals()->getSize();

       OSG::Int32 colors_size = 0;
       if(geo->getColors() != NullFC)
           colors_size = geo->getColors()->getSize();

       OSG::Int32 secondary_colors_size = 0;
       if(geo->getSecondaryColors() != NullFC)
           secondary_colors_size = geo->getSecondaryColors()->getSize();

       OSG::Int32 texccords_size = 0;
       if(geo->getTexCoords() != NullFC)
           texccords_size = geo->getTexCoords()->getSize();

       OSG::Int32 texccords1_size = 0;
       if(geo->getTexCoords1() != NullFC)
           texccords1_size = geo->getTexCoords1()->getSize();

       OSG::Int32 texccords2_size = 0;
       if(geo->getTexCoords2() != NullFC)
           texccords2_size = geo->getTexCoords2()->getSize();

       OSG::Int32 texccords3_size = 0;
       if(geo->getTexCoords3() != NullFC)
           texccords3_size = geo->getTexCoords3()->getSize();

       OSG::UInt32 pos_errors = 0;
       OSG::UInt32 norm_errors = 0;
       OSG::UInt32 col_errors = 0;
       OSG::UInt32 col2_errors = 0;
       OSG::UInt32 tex0_errors = 0;
       OSG::UInt32 tex1_errors = 0;
       OSG::UInt32 tex2_errors = 0;
       OSG::UInt32 tex3_errors = 0;

       OSG::PrimitiveIterator it;
       for(it = geo->beginPrimitives(); it != geo->endPrimitives(); ++it)
       {
           for(OSG::UInt32 v=0; v < it.getLength(); ++v)
           {
               if(it.getPositionIndex(v) >= positions_size)
                   ++pos_errors;
               if(it.getNormalIndex(v) >= normals_size)
                   ++norm_errors;
               if(it.getColorIndex(v) >= colors_size)
                   ++col_errors;
               if(it.getSecondaryColorIndex(v) >= secondary_colors_size)
                   ++col2_errors;
               if(it.getTexCoordsIndex(v) >= texccords_size)
                   ++tex0_errors;
               if(it.getTexCoordsIndex1(v) >= texccords1_size)
                   ++tex1_errors;
               if(it.getTexCoordsIndex2(v) >= texccords2_size)
                   ++tex2_errors;
               if(it.getTexCoordsIndex3(v) >= texccords3_size)
                   ++tex3_errors;
           }
       }


       if((norm_errors > 0) && mRepair)
       {
           norm_errors = 0;
           if(mVerbose) std::cout << "removed corrupted normals!\n";
           beginEditCP(geo);
               geo->setNormals(NullFC);
           endEditCP(geo);
       }

       if((col_errors > 0) && mRepair)
       {
           col_errors = 0;
           if(mVerbose) std::cout << "removed corrupted colors!\n";
           beginEditCP(geo);
               geo->setColors(NullFC);
           endEditCP(geo);
       }

       if((tex0_errors > 0) && mRepair)
       {
           tex0_errors = 0;
           if(mVerbose) std::cout << "removed corrupted tex coords0!\n";
           beginEditCP(geo);
               geo->setTexCoords(NullFC);
           endEditCP(geo);
       }

       mNumErrors += (pos_errors + norm_errors + col_errors +
                       col2_errors + tex0_errors + tex1_errors +
                       tex2_errors + tex3_errors);

       // found some errors.
       if(mNumErrors > start_errors)
       { mCorruptedGeos.push_back(geo); }

       // ok we found no errors now check for missing index map.
       bool need_repair(false);
       if(!verifyIndexMap(geo, need_repair))
       {
           if(need_repair)
           { std::cerr << "verifyGeometryCB : added missing index map!"; }
           else
           { std::cerr << "verifyGeometryCB : couldn't add missing index map!\n"; }
       }

       return OSG::Action::Continue;
   }


   bool verifyIndexMap(GeometryPtr &geo, bool &repair)
   {
       repair = false;

       if(geo == NullFC)
           return true;

       if(geo->getIndices() == NullFC)
           return true;

       if(!geo->getIndexMapping().empty())
           return true;

       if(geo->getPositions() == NullFC)
           return true;

       OSG::UInt32 positions_size = geo->getPositions()->getSize();

       OSG::UInt32 normals_size = 0;
       if(geo->getNormals() != NullFC)
           normals_size = geo->getNormals()->getSize();

       OSG::UInt32 colors_size = 0;
       if(geo->getColors() != NullFC)
           colors_size = geo->getColors()->getSize();

       OSG::UInt32 secondary_colors_size = 0;
       if(geo->getSecondaryColors() != NullFC)
           secondary_colors_size = geo->getSecondaryColors()->getSize();

       OSG::UInt32 texccords_size = 0;
       if(geo->getTexCoords() != NullFC)
           texccords_size = geo->getTexCoords()->getSize();

       OSG::UInt32 texccords1_size = 0;
       if(geo->getTexCoords1() != NullFC)
           texccords1_size = geo->getTexCoords1()->getSize();

       OSG::UInt32 texccords2_size = 0;
       if(geo->getTexCoords2() != NullFC)
           texccords2_size = geo->getTexCoords2()->getSize();

       OSG::UInt32 texccords3_size = 0;
       if(geo->getTexCoords3() != NullFC)
           texccords3_size = geo->getTexCoords3()->getSize();

       /*
       printf("sizes: %u %u %u %u %u %u %u %u\n", positions_size, normals_size,
                                      colors_size, secondary_colors_size,
                                      texccords_size, texccords1_size,
                                      texccords2_size, texccords3_size);
       */
       if((positions_size == normals_size || normals_size == 0) &&
          (positions_size == colors_size || colors_size == 0) &&
          (positions_size == secondary_colors_size || secondary_colors_size == 0) &&
          (positions_size == texccords_size || texccords_size == 0) &&
          (positions_size == texccords1_size || texccords1_size == 0) &&
          (positions_size == texccords2_size || texccords2_size == 0) &&
          (positions_size == texccords3_size || texccords3_size == 0))
       {
           OSG::UInt16 indexmap = 0;
           if(positions_size > 0)
               indexmap |= Geometry::MapPosition;
           if(normals_size > 0)
               indexmap |= Geometry::MapNormal;
           if(colors_size > 0)
               indexmap |= Geometry::MapColor;
           if(secondary_colors_size > 0)
               indexmap |= Geometry::MapSecondaryColor;
           if(texccords_size > 0)
               indexmap |= Geometry::MapTexCoords;
           if(texccords1_size > 0)
               indexmap |= Geometry::MapTexCoords1;
           if(texccords2_size > 0)
               indexmap |= Geometry::MapTexCoords2;
           if(texccords3_size > 0)
               indexmap |= Geometry::MapTexCoords3;

           beginEditCP(geo, Geometry::IndexMappingFieldMask);
               geo->getIndexMapping().push_back(indexmap);
           endEditCP(geo, Geometry::IndexMappingFieldMask);
           repair = true;
           return false;
       }
       else
       {
           return false;
       }

       return true;
   }


   /*!
    * \brief Replaces corrupted geometrie nodes with group nodes.
    * \return true if something was repaired.
    */
   bool repairGeometry()
   {
      if (!mCorruptedGeos.empty())
      {
         if (mVerbose)
         { std::cout << "Repairing corrupted geos:" << std::endl; }

         for (OSG::UInt32 i=0;i<mCorruptedGeos.size();++i)
         {
            // now replace corrupted geometry core with a group core.
            for (OSG::UInt32 j=0;j<mCorruptedGeos[i]->getParents().size();++j)
            {
               NodePtr parent = mCorruptedGeos[i]->getParents()[j];
               if (parent != NullFC)
               {
                  std::string nname;
                  if(OSG::getName(parent) != NULL)
                  { nname = OSG::getName(parent); }
                  if (mVerbose)
                  { std::cout << "Removing corrupted geom from node: " << nname << std::endl; }
                  nname += "_CORRUPTED";
                  OSG::setName(parent, nname.c_str());
                  beginEditCP(parent, Node::CoreFieldMask);
                  parent->setCore(Group::create());
                  endEditCP(parent, Node::CoreFieldMask);
               }
            }
         }
         return true;
      }

      return false;
   }

protected:
   bool        mVerbose;   /**< If true, output verbose information about the verify. */
   bool        mRepair;    /**< If true, attempt to repair things. */

   OSG::UInt32 mNumErrors;  /**< The running total of errors found. */

   std::vector<OSG::GeometryPtr> mCorruptedGeos;
   std::vector<OSG::NodePtr>     mCorruptedNodes;

};


class StatusPanel
{
public:
   StatusPanel();

   /** Initialize scene graph, fonts and everything else that is used. */
   void initialize();

   OSG::NodeRefPtr getPanelRoot()
   { return mRootPanelNode; }

   void setHeaderTitle(std::string txt);
   void setCenterTitle(std::string txt);
   void setBottomTitle(std::string txt);

   /** The text for the header. */
   void setHeaderText(std::string header);

   /** Set text for the control section of the panel. */
   void setControlText(std::string text);

   /** Add another message to the status panel. */
   void addStatusMessage(std::string msg);

   void updatePanelScene();

protected:
   inf::UiBuilder        mBuilder;

   OSG::NodeRefPtr       mRootPanelNode;
   OSG::NodeRefPtr       mPanelGeomNode;
   OSG::GeometryRefPtr   mPanelGeomCore;
   OSG::NodeRefPtr       mTextGeomNode;
   OSG::GeometryRefPtr   mTextGeomCore;

   inf::UiBuilder::Font* mFont;

   std::string    mHeaderTitle;
   std::string    mCenterTitle;
   std::string    mBottomTitle;

   std::string    mHeaderText;
   std::string    mCenterText;
   std::deque<std::string> mStatusLines;

   /** Panel sizes are in OpenSG units.
    * Everything else is in normalized percentages or discrete num of's (ie. num lines).
    * Thus all real sizes are derived based on the panel size set.
    * This "should" make it possible to resize the panel and get good results.
    */
   float    mPanWidth, mPanHeight;  /**< Panel size in OpenSG units. */
   float    mBorderWidth;           /**< Width of the border in real units. */

   // -- Percentages -- //
   float    mTitleHeight;           /**< Height to make the titles. */

   float    mHeaderHeight;          /**< Size of the header section (percentage). */
   float    mStatusHeight;          /**< Size of the status section (percentage). */

   unsigned mStatusHistorySize;    /**< Number of status lines to keep around. */
   float    mStatusTextHeight;     /**< Fixed height (in OpenSG coords) of status text. */


   // Colors/theming
   OSG::Color3f   mBgColor;      /**< Color of the background. */
   float          mBgAlpha;
   OSG::Color3f   mBorderColor;
   OSG::Color3f   mTitleColor;
   OSG::Color3f   mTextColor;

};

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

   OSG::ClipPlaneChunkPtr clip_right = ClipPlaneChunk::create();
   beginEditCP(clip_right);
   clip_right->setEquation(Vec4f(-1,0,0,mPanWidth));      // X clip plane <= right size
   clip_right->setEnable(true);
   clip_right->setBeacon(mTextGeomNode);
   endEditCP(clip_right);

   OSG::ClipPlaneChunkPtr clip_bottom = ClipPlaneChunk::create();
   beginEditCP(clip_bottom);
   clip_bottom->setEquation(Vec4f(0,1,0,0));         // Y clip plane on Y>=0
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



void update()
{
   static unsigned frame(0);
   frame += 1;

   gOverlayTextStream << "frame: " << frame << "\n";

   std::string overlay_text = gOverlayTextStream.str();
   std::vector<std::string> overlay_lines;
   splitStr(overlay_text,"\n",std::back_inserter(overlay_lines));
}

void printFontFamilies()
{
   std::vector<std::string> families;
   OSG::TextFaceFactory::the().getFontFamilies(families);  // Get list of all families

   std::cout << "--- Font Families ---\n";
   for(unsigned i=0; i<families.size(); ++i)
   {
      std::cout << families[i] << std::endl;
   }
   std::cout << "---------------------\n";
}

int main(int argc, char* argv[])
{
   // Init OSG and glut.
    OSG::osgInit(argc,argv);
    int winid = setupGLUT(&argc, argv);
    OSG::GLUTWindowPtr gwin = OSG::GLUTWindow::create();
    gwin->setId(winid);
    gwin->init();

    printFontFamilies();

    // load the scene
    OSG::NodePtr scene = OSG::Node::create();
    scene->setCore(OSG::Group::create());

    OSG::NodePtr ui_group = OSG::Node::create();
    OSG::setName(ui_group, "ui_group");
    ui_group->setCore(OSG::Group::create());

    OSG::NodePtr panel_node = OSG::Node::create();
    OSG::setName(panel_node, "panel_node");

    //
#if 0
    inf::UiBuilder builder;

    OSG::Color3f white(0.4,0.4,0.4);
    OSG::GeometryPtr pan_geom = builder.createGeomGeo();

    OSG::Color3f bg_color(0.4, 0.4, 0.4);
    float bg_alpha(0.4);

    /*
    builder.buildRoundedRectangle(pan_geom, OSG::Color3f(0,1,0), OSG::Pnt2f(0,15), OSG::Pnt2f(10,20), 0.25, 0.75, 8, false, 0, -0.5, 1.0);
    builder.buildRoundedRectangle(pan_geom, OSG::Color3f(0,0.3,0.4), OSG::Pnt2f(15,0), OSG::Pnt2f(25,10), 0.5, 0.75, 8, true, 0, 0, 1.0);

    builder.buildRoundedRectangle(pan_geom, white,    OSG::Pnt2f(0,0), OSG::Pnt2f(10,10), 0.3, 0.6, 8, false, 0.2, -0.1, 1.0);
    builder.buildRoundedRectangle(pan_geom, bg_color, OSG::Pnt2f(0,0), OSG::Pnt2f(10,10), 0.0, 1.0, 8, true,  0,      0, bg_alpha);
    */
    builder.buildRectangle(pan_geom, white, OSG::Pnt2f(0,0), OSG::Pnt2f(10,10), -0.1,-0.1);

    panel_node->setCore(pan_geom);
    ui_group->addChild(panel_node);

    OSG::Color3f text_color(0,1,0);
    OSG::GeometryPtr text_geom = builder.createTextGeom();
    inf::UiBuilder::Font font("SANS", OSG::TextFace::STYLE_PLAIN, 64);

    builder.buildText(text_geom, font, "At 0,0\nText", OSG::Vec2f(0,0), text_color, 1.0f, 1.0f);
    builder.addText(text_geom, font, "At 0,10\nText", OSG::Vec2f(0,10), OSG::Color3f(0,1,1), 1.0f, 1.0f);

    OSG::NodePtr text_node = OSG::Node::create();
    text_node->setCore(text_geom);
    ui_group->addChild(text_node);

    scene->addChild(ui_group);
#endif

#if 0
    // Normal geometry
    OSG::NodePtr norm_node = OSG::calcVertexNormalsGeo(pan_geom, 0.2f);
    OSG::GeometryPtr norm_geo = OSG::GeometryPtr::dcast(norm_node->getCore());
    OSG::SimpleMaterialPtr norm_mat = SimpleMaterial::create();

    norm_mat->setLit(false);
    norm_mat->setDiffuse(OSG::Color3f(1,1,1));
    norm_geo->setMaterial(norm_mat);

    ui_group->addChild(norm_node);
#endif

    // Status panel
   ///*
    StatusPanel status_panel;
    status_panel.initialize();


    std::stringstream status_stream;

    for (unsigned i=0;i<20;++i)
    {
       status_stream << "Output number: " << i << std::endl;
       status_stream << "   more info here." << std::endl;
       status_panel.addStatusMessage(status_stream.str());
       status_stream.str("");
    }

    status_stream << "Initialized StatusPanel." << std::endl;
    status_panel.addStatusMessage(status_stream.str());
    status_stream.str("");

    status_stream << "StatusPanel::Init: Called with param x: this,that.  Then it executed a really long line of text." << std::endl;
    status_panel.addStatusMessage(status_stream.str());
    status_stream.str("");

    status_stream << "Current window status:\n"
                  << "    size: 10:\n"
                  << "    pos: 10,10\n"
                  << "    open: yes\n" << std::endl;
    status_panel.addStatusMessage(status_stream.str());
    status_stream.str("");

    status_panel.updatePanelScene();

    scene->addChild(status_panel.getPanelRoot());
    //*/


    VerifyTraverser verifier;
    verifier.setVerbose(true);
    verifier.setRepair(false);
    verifier.verify(scene);

    OSG::VerifyGeoGraphOp verify_op("verify", false);
    bool verify = verify_op.traverse(scene);
    if(!verify)
    {
       std::cout << "Graph has problems." << std::endl;
    }

    mgr = new OSG::SimpleSceneManager;

    // Tell the manager about the window and scene
    mgr->setWindow(gwin );
    mgr->setRoot(scene);

   // Start it up
    mgr->showAll();
    glutMainLoop();

    return 0;
}

// ----- GLUT callback functions ---- //

// redraw the window
void display(void)
{
   update();

    mgr->idle();
    mgr->redraw();
}

// react to size changes
void reshape(int w, int h)
{
    mgr->resize(w, h);
    glutPostRedisplay();
}

// react to mouse button presses
void mouse(int button, int state, int x, int y)
{

    if (state)
    {   mgr->mouseButtonRelease(button, x, y); }
    else
    {   mgr->mouseButtonPress(button, x, y); }

    glutPostRedisplay();
}

// react to mouse motions with pressed buttons
void motion(int x, int y)
{

    mgr->mouseMove(x, y);
    glutPostRedisplay();
}

// react to keys
void keyboard(unsigned char k, int , int )
{
   switch (k)
   {
   case 27:
      {
         OSG::osgExit();
         exit(0);
      }
      break;

   case 'd':
      OSG::SceneFileHandler::the().write(mgr->getRoot(),"dump_scene.osb");
      std::cout << "Wrote out scene: dump_scene.osb" << std::endl;
      break;

   case 't':
      mgr->setNavigationMode(OSG::Navigator::TRACKBALL);
      break;

   case 'l':
      mgr->setHeadlight(!mgr->getHeadlightState());
      std::cout << "Set headlight: " << mgr->getHeadlightState() << std::endl;
      break;
   case 'z':
      glPolygonMode( GL_FRONT_AND_BACK, GL_POINT);
      std::cerr << "PolygonMode: Point." << std::endl;
      break;
   case 'x':   glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
      std::cerr << "PolygonMode: Line." << std::endl;
      break;
   case 'c':   glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
      std::cerr << "PolygonMode: Fill." << std::endl;
      break;
   }
}

void initgl(void)
{
   glClearColor(0.1, 0.0, 0.1, 0.0);
   glShadeModel(GL_SMOOTH);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
}

// setup the GLUT library which handles the windows for us
int setupGLUT(int *argc, char *argv[])
{
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);


    int winid = glutCreateWindow("IOV UI Test");

    std::cout << "Depth size: " << glutGet(GLUT_WINDOW_DEPTH_SIZE) << std::endl;

    initgl();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);

    return winid;
}
