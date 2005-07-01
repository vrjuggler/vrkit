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

#include <OpenSG/OSGTextTXFFace.h>
#include <OpenSG/OSGTextTXFGlyph.h>
#include <OpenSG/OSGTextFaceFactory.h>
#include <OpenSG/OSGTextLayoutParam.h>
#include <OpenSG/OSGTextLayoutResult.h>

#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGImage.h>

#include <OpenSG/OSGGeoFunctions.h>

#include <OpenSG/OSGVerifyGeoGraphOp.h>

#include <IOV/UiBuilder.h>
#include <sstream>


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

   float    mStatusHeight;          /**< Size of the status section. */
   float    mHeaderHeight;          /**< Size of the header section. */

   unsigned mStatusHistorySize;  /**< Number of status lines to keep around. */
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
   mStatusTextHeight = 0.1f;

   mBgColor.setValuesRGB(0.5, 0.5, 0.5);
   mBgAlpha = 0.5f;
   mBorderColor.setValuesRGB(1,1,1);
   mTitleColor.setValuesRGB(1,0.5,0);
   mTextColor.setValuesRGB(1,1,1);

   mHeaderTitle = "Header";
   mCenterTitle = "Center";
   mBottomTitle = "Bottom";

   mHeaderText  = "Header\nText";
   mCenterText  = "Center\nText\nhere";
}


void StatusPanel::initialize()
{

   mRootPanelNode = OSG::Node::create();
   mPanelGeomNode = OSG::Node::create();
   mPanelGeomCore = mBuilder.createGeomGeo();

   mTextGeomNode = OSG::Node::create();
   mTextGeomCore = mBuilder.createTextGeom();

   OSG::CPEditor rpne(mRootPanelNode);
   OSG::CPEditor pgne(mPanelGeomNode);
   OSG::CPEditor pgce(mPanelGeomCore);
   OSG::CPEditor tgne(mTextGeomNode);
   OSG::CPEditor tgce(mTextGeomCore);

   mPanelGeomNode->setCore(mPanelGeomCore);
   mTextGeomNode->setCore(mTextGeomCore);

   mRootPanelNode->addChild(mPanelGeomNode);
   mRootPanelNode->addChild(mTextGeomNode);

   mFont = new inf::UiBuilder::Font("SANS", OSG::TextFace::STYLE_PLAIN, 64);
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

   mBuilder.buildRoundedRectangle(mPanelGeomCore, mBorderColor, panel_ll, panel_ur, inner_rad, inner_rad+mBorderWidth,
                                 num_segs, false, front_depth, back_depth, 1.0);
   mBuilder.buildRoundedRectangle(mPanelGeomCore, mBgColor,     panel_ll, panel_ur, 0.0, inner_rad+(mBorderWidth*2),
                                 num_segs, true,  0,      0, mBgAlpha);

   const float text_spacing(0.7);
   float abs_title_height = mTitleHeight*mPanHeight;
   const float title_indent(0.1 * mPanWidth);
   OSG::Vec2f header_title_pos(title_indent,mPanHeight-abs_title_height);

   mBuilder.buildText(mTextGeomCore.get(), *mFont, mHeaderTitle, header_title_pos, mTitleColor, abs_title_height, text_spacing);

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

    inf::UiBuilder builder;
    OSG::NodePtr ui_group = OSG::Node::create();
    ui_group->setCore(OSG::Group::create());

    OSG::NodePtr panel_node = OSG::Node::create();

    //
    /*
    OSG::Color3f white(1,1,1);
    OSG::GeometryPtr pan_geom = builder.createGeomGeo();

    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.7, 0.2), OSG::Pnt2f(-5,4), 0.5, 2.0, 17, 0, gmtl::Math::PI_OVER_2, 0.5, -0.5, true, 1.0f);
    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.7, 0.7), OSG::Pnt2f(-7,3), 0, 1.7, 17, 0, gmtl::Math::PI_OVER_2, 0.5, -0.5, true, 1.0f);
    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.2, 0.8), OSG::Pnt2f(-5,-4),0.0, 1.5, 32, 0, gmtl::Math::TWO_PI, 0.5, -0.5, true, 1.0f);
    builder.buildDisc(pan_geom,  OSG::Color3f(0.5, 0.0, 0.8), OSG::Pnt2f(-8,0), 0.0, 1.5, 32, 0, gmtl::Math::PI *0.75 , 0, 0, true, 1.0f);

    OSG::Color3f bg_color(0.4, 0.4, 0.4);
    float bg_alpha(0.4);

    builder.buildRoundedRectangle(pan_geom, OSG::Color3f(0,1,0), OSG::Pnt2f(0,15), OSG::Pnt2f(10,20), 0.25, 0.75, 8, false, 0, -0.5, 1.0);
    builder.buildRoundedRectangle(pan_geom, OSG::Color3f(0,0.3,0.4), OSG::Pnt2f(15,0), OSG::Pnt2f(25,10), 0.5, 0.75, 8, true, 0, 0, 1.0);

    builder.buildRoundedRectangle(pan_geom, white,    OSG::Pnt2f(0,0), OSG::Pnt2f(10,10), 0.3, 0.6, 8, false, 0.2, -0.1, 1.0);
    builder.buildRoundedRectangle(pan_geom, bg_color, OSG::Pnt2f(0,0), OSG::Pnt2f(10,10), 0.0, 1.0, 8, true,  0,      0, bg_alpha);

    panel_node->setCore(pan_geom);
    ui_group->addChild(panel_node);

#ifdef 0
    // Normal geometry
    OSG::NodePtr norm_node = OSG::calcVertexNormalsGeo(pan_geom, 0.2f);
    OSG::GeometryPtr norm_geo = OSG::GeometryPtr::dcast(norm_node->getCore());
    OSG::SimpleMaterialPtr norm_mat = SimpleMaterial::create();

    norm_mat->setLit(false);
    norm_mat->setDiffuse(OSG::Color3f(1,1,1));
    norm_geo->setMaterial(norm_mat);

    ui_group->addChild(norm_node);
#endif

    OSG::Color3f text_color(1,1,1);
    OSG::GeometryPtr text_geom = builder.createTextGeom();
    inf::UiBuilder::Font font("SANS", OSG::TextFace::STYLE_PLAIN, 64);
    builder.buildText(text_geom, font, "This is\na\ntest.", OSG::Vec2f(3,8), text_color, 1.0f, 1.0f);
    builder.addText(text_geom, font, "More text here.\nIn YELLOW!!!", OSG::Vec2f(2,5), OSG::Color3f(1,1,0), 1.0f, 1.0f);

    OSG::NodePtr text_node = OSG::Node::create();
    text_node->setCore(text_geom);
    ui_group->addChild(text_node);

    scene->addChild(ui_group);
    */

    // Status panel
    StatusPanel status_panel;
    status_panel.initialize();
    status_panel.updatePanelScene();

    scene->addChild(status_panel.getPanelRoot());


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
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    int winid = glutCreateWindow("IOV UI Test");

    initgl();
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);

    return winid;
}
