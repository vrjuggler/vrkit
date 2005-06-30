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
   /** The text for the header. */
   void setHeaderText(std::string header);

   /** Set text for the control section of the panel. */
   void setControlText(std::string text);

   /** Add another message to the status panel. */
   void addStatusMessage(std::string msg);

protected:
   void updatePanelScene();

protected:

};

void StatusPanel::setHeaderText(std::string header)
{
}

void StatusPanel::setControlText(std::string text)
{

}

void StatusPanel::addStatusMessage(std::string msg)
{

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
    OSG::Color3f white(1,1,1);

    OSG::GeometryPtr pan_geom = builder.createGeomGeo();

    builder.buildRectangle(pan_geom, white, OSG::Pnt2f(1.0,1.0), OSG::Pnt2f(1.5,9.0), 0.3, -0.2);  // left
    builder.buildRectangle(pan_geom, white, OSG::Pnt2f(8.5,1.0), OSG::Pnt2f(9.0,9.0), 0.3, -0.2);  // right
    builder.buildRectangle(pan_geom, white, OSG::Pnt2f(1.0,8.5), OSG::Pnt2f(9.0,9.0), 0.3, -0.2);  // top
    builder.buildRectangle(pan_geom, white, OSG::Pnt2f(1.0,1.0), OSG::Pnt2f(9.0,1.5), 0.3, -0.2);  // bottom

    /*
    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.7, 0.2), OSG::Pnt2f(-5,4),
                      0.5, 2.0, 17, 0, gmtl::Math::PI_OVER_2, 0.5, -0.5, true, 1.0f);

    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.7, 0.7), OSG::Pnt2f(-7,3),
                      0, 1.7, 17, 0, gmtl::Math::PI_OVER_2, 0.5, -0.5, true, 1.0f);

    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.2, 0.8), OSG::Pnt2f(-5,-4),
                      0.0, 1.5, 32, 0, gmtl::Math::TWO_PI, 0.5, -0.5, true, 1.0f);

   */
    /*
    builder.buildDisc(pan_geom,  OSG::Color3f(0.5, 0.0, 0.8), OSG::Pnt2f(-8,0),
                      0.0, 1.5, 32, 0, gmtl::Math::PI * 1.5, 0, 0, true, 1.0f);
   */

    /*
    OSG::Color3f bg_color(0.7, 0.7, 0.7);
    float bg_alpha(0.4);
    float q1(gmtl::Math::PI_OVER_2), q2(gmtl::Math::PI), q3(gmtl::Math::PI+gmtl::Math::PI_OVER_2),
          q4(gmtl::Math::TWO_PI);
    builder.buildRectangle(pan_geom, bg_color, OSG::Pnt2f(0,1), OSG::Pnt2f(10,9), 0,0, bg_alpha);
    builder.buildRectangle(pan_geom, bg_color, OSG::Pnt2f(1,0), OSG::Pnt2f(9,1),  0,0, bg_alpha);
    builder.buildRectangle(pan_geom, bg_color, OSG::Pnt2f(1,9), OSG::Pnt2f(9,10), 0,0, bg_alpha);

    builder.buildDisc(pan_geom, bg_color, OSG::Pnt2f(9,9), 0, 1, 16,  0, q1,0,0,false,bg_alpha);  // ur
    builder.buildDisc(pan_geom, bg_color, OSG::Pnt2f(1,9), 0, 1, 16, q1, q2,0,0,false,bg_alpha);  // ul
    builder.buildDisc(pan_geom, bg_color, OSG::Pnt2f(1,1), 0, 1, 16, q2, q3,0,0,false,bg_alpha);  // ll
    builder.buildDisc(pan_geom, bg_color, OSG::Pnt2f(9,1), 0, 1, 16, q3, q4,0,0,false,bg_alpha);  // lr
    */


    panel_node->setCore(pan_geom);
    ui_group->addChild(panel_node);

    OSG::Color3f text_color(1,1,1);
    OSG::GeometryPtr text_geom = builder.createTextGeom();
    inf::UiBuilder::Font font("SANS", OSG::TextFace::STYLE_PLAIN, 64);
    builder.buildText(text_geom, font, "This is\na\ntest.", OSG::Vec2f(4,6), text_color, 1.0f, 1.0f);
    builder.addText(text_geom, font, "More text here.\nIn YELLOW!!!", OSG::Vec2f(7,3), OSG::Color3f(1,1,0), 1.0f, 1.0f);

    OSG::NodePtr text_node = OSG::Node::create();
    text_node->setCore(text_geom);
    ui_group->addChild(text_node);

    scene->addChild(ui_group);

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
