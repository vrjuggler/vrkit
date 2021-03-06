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

// Test for UI code
//
// Just use simple scene manager and create a ui
// to display locally.
//

#include <sstream>
#include <algorithm>
#include <vpr/vprConfig.h>
#include <vpr/Util/Assert.h>

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

#include <vrkit/UiBuilder.h>

// XXX: These include paths stink.
#include "../../plugins/StatusPanelPlugin/StatusPanel.h"
#include "../../plugins/StatusPanelPlugin/StatusPanelViewOriginal.h"


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
       if(node == OSG::NullFC)
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
                OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<
                  OSG::Action::ResultE, VerifyTraverser, OSG::NodePtr
                >(this, &VerifyTraverser::verifyCB));

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
         if(OSG::GeometryPtr::dcast(node_core) != OSG::NullFC)
         {
            return verifyGeometryCB(node);
         }
      }

      return OSG::Action::Continue;

   }

protected:  // -- Geom --- //



   /** Verify geometry method. */
   OSG::Action::ResultE verifyGeometryCB(OSG::NodePtr &node)
   {
       OSG::GeometryPtr geo = OSG::GeometryPtr::dcast(node->getCore());

       if(geo == OSG::NullFC)
           return OSG::Action::Continue;

       if(geo->getPositions() == OSG::NullFC)
           return OSG::Action::Continue;

       OSG::UInt32 start_errors = mNumErrors;

       OSG::Int32 positions_size = geo->getPositions()->getSize();

       OSG::Int32 normals_size = 0;
       if(geo->getNormals() != OSG::NullFC)
           normals_size = geo->getNormals()->getSize();

       OSG::Int32 colors_size = 0;
       if(geo->getColors() != OSG::NullFC)
           colors_size = geo->getColors()->getSize();

       OSG::Int32 secondary_colors_size = 0;
       if(geo->getSecondaryColors() != OSG::NullFC)
           secondary_colors_size = geo->getSecondaryColors()->getSize();

       OSG::Int32 texccords_size = 0;
       if(geo->getTexCoords() != OSG::NullFC)
           texccords_size = geo->getTexCoords()->getSize();

       OSG::Int32 texccords1_size = 0;
       if(geo->getTexCoords1() != OSG::NullFC)
           texccords1_size = geo->getTexCoords1()->getSize();

       OSG::Int32 texccords2_size = 0;
       if(geo->getTexCoords2() != OSG::NullFC)
           texccords2_size = geo->getTexCoords2()->getSize();

       OSG::Int32 texccords3_size = 0;
       if(geo->getTexCoords3() != OSG::NullFC)
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
           OSG::beginEditCP(geo);
               geo->setNormals(OSG::NullFC);
           OSG::endEditCP(geo);
       }

       if((col_errors > 0) && mRepair)
       {
           col_errors = 0;
           if(mVerbose) std::cout << "removed corrupted colors!\n";
           OSG::beginEditCP(geo);
               geo->setColors(OSG::NullFC);
           OSG::endEditCP(geo);
       }

       if((tex0_errors > 0) && mRepair)
       {
           tex0_errors = 0;
           if(mVerbose) std::cout << "removed corrupted tex coords0!\n";
           OSG::beginEditCP(geo);
               geo->setTexCoords(OSG::NullFC);
           OSG::endEditCP(geo);
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


   bool verifyIndexMap(OSG::GeometryPtr &geo, bool &repair)
   {
       repair = false;

       if(geo == OSG::NullFC)
           return true;

       if(geo->getIndices() == OSG::NullFC)
           return true;

       if(!geo->getIndexMapping().empty())
           return true;

       if(geo->getPositions() == OSG::NullFC)
           return true;

       OSG::UInt32 positions_size = geo->getPositions()->getSize();

       OSG::UInt32 normals_size = 0;
       if(geo->getNormals() != OSG::NullFC)
           normals_size = geo->getNormals()->getSize();

       OSG::UInt32 colors_size = 0;
       if(geo->getColors() != OSG::NullFC)
           colors_size = geo->getColors()->getSize();

       OSG::UInt32 secondary_colors_size = 0;
       if(geo->getSecondaryColors() != OSG::NullFC)
           secondary_colors_size = geo->getSecondaryColors()->getSize();

       OSG::UInt32 texccords_size = 0;
       if(geo->getTexCoords() != OSG::NullFC)
           texccords_size = geo->getTexCoords()->getSize();

       OSG::UInt32 texccords1_size = 0;
       if(geo->getTexCoords1() != OSG::NullFC)
           texccords1_size = geo->getTexCoords1()->getSize();

       OSG::UInt32 texccords2_size = 0;
       if(geo->getTexCoords2() != OSG::NullFC)
           texccords2_size = geo->getTexCoords2()->getSize();

       OSG::UInt32 texccords3_size = 0;
       if(geo->getTexCoords3() != OSG::NullFC)
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
               indexmap |= OSG::Geometry::MapPosition;
           if(normals_size > 0)
               indexmap |= OSG::Geometry::MapNormal;
           if(colors_size > 0)
               indexmap |= OSG::Geometry::MapColor;
           if(secondary_colors_size > 0)
               indexmap |= OSG::Geometry::MapSecondaryColor;
           if(texccords_size > 0)
               indexmap |= OSG::Geometry::MapTexCoords;
           if(texccords1_size > 0)
               indexmap |= OSG::Geometry::MapTexCoords1;
           if(texccords2_size > 0)
               indexmap |= OSG::Geometry::MapTexCoords2;
           if(texccords3_size > 0)
               indexmap |= OSG::Geometry::MapTexCoords3;

           OSG::beginEditCP(geo, OSG::Geometry::IndexMappingFieldMask);
               geo->getIndexMapping().push_back(indexmap);
           OSG::endEditCP(geo, OSG::Geometry::IndexMappingFieldMask);
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
               OSG::NodePtr parent = mCorruptedGeos[i]->getParents()[j];
               if (parent != OSG::NullFC)
               {
                  std::string nname;
                  if(OSG::getName(parent) != NULL)
                  { nname = OSG::getName(parent); }
                  if (mVerbose)
                  { std::cout << "Removing corrupted geom from node: " << nname << std::endl; }
                  nname += "_CORRUPTED";
                  OSG::setName(parent, nname.c_str());
                  OSG::beginEditCP(parent, OSG::Node::CoreFieldMask);
                     parent->setCore(OSG::Group::create());
                  OSG::endEditCP(parent, OSG::Node::CoreFieldMask);
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
    vrkit::UiBuilder builder;

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
    vrkit::UiBuilder::Font font("SANS", OSG::TextFace::STYLE_PLAIN, 64);

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
    // Use feet as the unit.
    vrkit::StatusPanel status_panel;
    vrkit::StatusPanelViewOriginal status_panel_view;
    status_panel_view.initialize(3.28f, &status_panel);


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

    status_panel_view.update();

    scene->addChild(status_panel_view.getPanelRoot());
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

    std::cout << "Near: " << mgr->getCamera()->getNear() << " Far: " << mgr->getCamera()->getFar() << std::endl;
    mgr->getCamera()->setNear(1.0);
    mgr->getCamera()->setFar(40.0f);
    std::cout << "Near: " << mgr->getCamera()->getNear() << " Far: " << mgr->getCamera()->getFar() << std::endl;

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

    int winid = glutCreateWindow("vrkit UI Test");

    std::cout << "Depth size: " << glutGet(GLUT_WINDOW_DEPTH_SIZE)
              << std::endl;

    initgl();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);

    return winid;
}
