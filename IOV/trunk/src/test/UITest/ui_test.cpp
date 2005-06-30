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


/** Little wrapper for holding text data. */
class TextStuff
{
public:
   TextStuff()
      : mFace(NULL),
        mFamilyName("SANS"),
        mNextFamily(0),
        mStyle(OSG::TextFace::STYLE_PLAIN),
        mTextGap(1),
        mTextureWidth(1024),
        mFaceSize(46),
        mLineSpacing(1.0f),
        mMaxExtent(0.0f),
        mGeoScale(1.0f)
   {;}

   void setText(std::vector<std::string> lines)
   {
      mLines = lines;
      updateScene();
   }

   // Initialize the scene structures and get everything going
   void initialize()
   {
      OSG::TextFaceFactory::the().getFontFamilies(mFamilies);  // Get list of all families

      mRootNode = OSG::Node::create();
      mTextGeom = OSG::Geometry::create();
      mTextMat = OSG::ChunkMaterial::create();
      mTextureChunk = OSG::TextureChunk::create();

      OSG::CPEditor rne(mRootNode);
      OSG::CPEditor tge(mTextGeom);
      OSG::CPEditor tme(mTextMat);
      OSG::CPEditor tce(mTextureChunk);

      // XXX: Setup a default face to use

      // Setup defaults for the texture
      OSG::ImagePtr img = OSG::Image::create();         // Temporary image for now
      OSG::UChar8 data[] = {0,0,0, 50,50,50, 100,100,100, 255,255,255};

      beginEditCP(img);
         img->set( OSG::Image::OSG_RGB_PF, 2, 2, 1, 1, 1, 0, data);
      endEditCP(img);

      // -- Setup texture and materials -- //
      mTextureChunk->setImage(img);
      mTextureChunk->setWrapS(GL_CLAMP);
      mTextureChunk->setWrapT(GL_CLAMP);
      mTextureChunk->setMagFilter(GL_LINEAR);
      mTextureChunk->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
      mTextureChunk->setEnvMode(GL_MODULATE);
      mTextureChunk->setInternalFormat(GL_INTENSITY);

      OSG::MaterialChunkPtr mat_chunk = OSG::MaterialChunk::create();
      OSG::beginEditCP(mat_chunk);
      {
         mat_chunk->setAmbient (OSG::Color4f(1.f, 0.f, 0.f, 1.f));
         mat_chunk->setDiffuse (OSG::Color4f(1.f, 0.f, 0.f, 1.f));
         mat_chunk->setEmission(OSG::Color4f(0.f, 0.f, 0.f, 1.f));
         mat_chunk->setSpecular(OSG::Color4f(0.f, 0.f, 0.f, 1.f));
         mat_chunk->setShininess(0);
      }
      OSG::endEditCP(mat_chunk);

      mTextMat->addChunk(mTextureChunk);
      mTextMat->addChunk(mat_chunk);

      mTextGeom->setMaterial(mTextMat);
      mRootNode->setCore(mTextGeom);
   }

   void updateScene()
   {
      OSG::TextLayoutResult layout_result;
      OSG::TextLayoutParam layout_param;
      layout_param.maxExtend = mMaxExtent;
      layout_param.setLength(mMaxExtent);
      layout_param.spacing = mLineSpacing;

      mFace->layout(mLines, layout_param, layout_result);
      OSG::GeometryPtr geom_ptr = mTextGeom.get();

      OSG::Vec2f bounds = layout_result.textBounds;
      //std::cout << "Text bounds: " << bounds << std::endl;
      mFace->fillGeo(geom_ptr, layout_result, mGeoScale);
   }

   void updateFace()
   {
      // Try to create new face
       OSG::TextTXFParam param;
       param.textureWidth = mTextureWidth;
       param.gap = mTextGap;
       param.size = mFaceSize;

       OSG::TextTXFFace* new_face = OSG::TextTXFFace::create(mFamilyName, mStyle, param);
       if (NULL == new_face)
       {
          std::cerr << "ERROR: Failed to allocate face." << std::endl;
       }
       subRefP(mFace);
       mFace = new_face;
       addRefP(mFace);

       OSG::beginEditCP(mTextureChunk);
         OSG::ImagePtr face_image = mFace->getTexture();
         mTextureChunk->setImage(face_image);
       OSG::endEditCP(mTextureChunk);
   }

   /** Increment/decrement current texture size and regen face. */
   void incTextureSize(bool dec=false)
   {
      switch (mTextureWidth)
      {
      case 0:  mTextureWidth = (dec?0:16);  break;
      case 16:  mTextureWidth = (dec?0:32);  break;
      case 32:  mTextureWidth = (dec?16:64);  break;
      case 64:  mTextureWidth = (dec?32:128);  break;
      case 128:  mTextureWidth = (dec?64:256);  break;
      case 256:  mTextureWidth = (dec?128:512);  break;
      case 512:  mTextureWidth = (dec?256:1024);  break;
      case 1024:  mTextureWidth = (dec?512:1024);  break;
      }

      std::cout << "Setting mTextureWidth: " << mTextureWidth << std::endl;

      updateFace();
      updateScene();
   }

   void incFaceSize(bool inc=true)
   {
      if(inc)
         mFaceSize += 1;
      else if(0 != mFaceSize)
         mFaceSize -= 1;

      std::cout << "Set face size to: " << mFaceSize << std::endl;
      updateFace();
      updateScene();
   }

   /** Cycle to the next family in the set of available font families. */
   void goToNextFamily()
   {
      mFamilyName = mFamilies[mNextFamily];
      mNextFamily += 1;
      if(mNextFamily >= mFamilies.size())
      { mNextFamily = 0; }

      std::cout << "New family: " << mFamilyName << std::endl;
      updateFace();
      updateScene();
   }

   void incLineSpacing(bool inc=true)
   {
      float increment(0.05f);
      if(!inc)
      { increment *= -1.0f; }

      mLineSpacing += increment;

      if(mLineSpacing < 0.0f)
      {  mLineSpacing = 0.0f; }

      std::cout << "Set line spacing to: " << mLineSpacing << std::endl;
      updateScene();
   }

   void incMaxExtent(bool inc=true)
   {
      float increment(0.5f);
      if(!inc)
      { increment *= -1.0f; }

      mMaxExtent += increment;

      if(mMaxExtent < 0.0f)
      { mMaxExtent = 0.0f; }

      std::cout << "Set max extent to: " << mMaxExtent << std::endl;
      updateScene();
   }

   void incGeoScale(bool inc=true)
   {
      float increment(0.05f);
      if(!inc)
      { increment *= -1.0f; }

      mGeoScale += increment;

      if(mGeoScale < 0.0f)
      { mGeoScale = 0.0f; }

      std::cout << "Set geo scale to: " << mGeoScale << std::endl;
      updateScene();
   }

public:
   std::vector<std::string>   mLines;

   OSG::NodeRefPtr            mRootNode;     /**< Root node for text geom. */
   OSG::GeometryRefPtr        mTextGeom;     /**< Geom core for the text. */
   OSG::ChunkMaterialRefPtr   mTextMat;      /**< Material for the text geom. */
   OSG::TextureChunkRefPtr    mTextureChunk; /**< Texture chunk for the text material. */

   OSG::TextTXFFace*          mFace;
   std::string                mFamilyName;   /**< The name of the font family. */
   std::vector<std::string>   mFamilies;
   unsigned                   mNextFamily;   /**< Next text family to use. */
   OSG::TextFace::Style       mStyle;

   unsigned                   mTextGap;      /**< The gap between glyphs in pixels */
   unsigned                   mTextureWidth; /**< The width of the texture in pixels */
   unsigned                   mFaceSize;     /**< The "size" param of the face. */

   float                      mLineSpacing;  /**< Spacing to use in the layout. */
   float                      mMaxExtent;    /**< Maximum extent to use. */

   float                      mGeoScale;     /**< Scale for geometry. */
};

TextStuff  gTextStuff;

void update()
{
   static unsigned frame(0);
   frame += 1;

   gOverlayTextStream << "frame: " << frame << "\n";

   std::string overlay_text = gOverlayTextStream.str();
   std::vector<std::string> overlay_lines;
   splitStr(overlay_text,"\n",std::back_inserter(overlay_lines));

   gTextStuff.setText(overlay_lines);

   gOverlayTextStream.str(std::string(""));  // Clear it out

   gOverlayTextStream << "Hello\nWorld!!\n";

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
    builder.buildBox(pan_geom, white,OSG::Pnt2f(1,1), OSG::Pnt2f(2,9), 0.5f);  // left
    //builder.buildBox(pan_geom, white,OSG::Pnt2f(8,1), OSG::Pnt2f(9,9), 0.5f);  // right
    //builder.buildBox(pan_geom, white,OSG::Pnt2f(1,8), OSG::Pnt2f(9,9), 0.5f);  // top
    //builder.buildBox(pan_geom, white,OSG::Pnt2f(1,1), OSG::Pnt2f(9,2), 0.5f);  // bottom
    builder.buildRectangle(pan_geom, OSG::Color3f(0.7,0.7,0.7),
                           OSG::Pnt2f(0,0), OSG::Pnt2f(10,10), 0.4, true);

    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.7, 0.2), OSG::Pnt2f(-5,4),
                      0.5, 2.0, 17, 0, gmtl::Math::PI_OVER_2, 0.5, -0.5, true, 1.0f);

    builder.buildDisc(pan_geom,  OSG::Color3f(0.0, 0.2, 0.8), OSG::Pnt2f(-5,-4),
                      0.0, 1.5, 32, 0, gmtl::Math::TWO_PI, 0.5, -0.5, true, 1.0f);

    builder.buildDisc(pan_geom,  OSG::Color3f(0.5, 0.0, 0.8), OSG::Pnt2f(-8,0),
                      0.0, 1.5, 32, 0, gmtl::Math::TWO_PI, 0, 0, true, 1.0f);



    panel_node->setCore(pan_geom);
    ui_group->addChild(panel_node);

    OSG::GeometryPtr text_geom = builder.createTextGeom();
    inf::UiBuilder::Font font("SANS", OSG::TextFace::STYLE_PLAIN, 64);
    builder.buildText(text_geom, font, "This is\na\test.", 1.0f, 1.0f);
    OSG::NodePtr text_node = OSG::Node::create();
    text_node->setCore(text_geom);
    ui_group->addChild(text_node);

    scene->addChild(ui_group);

    // Setup text sample
    gTextStuff.initialize();
    gTextStuff.updateFace();
    gTextStuff.updateScene();
    //scene->addChild(gTextStuff.mRootNode);

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

   case '-':
      gTextStuff.incFaceSize(false);
      break;
   case '=':
      gTextStuff.incFaceSize(true);
      break;

   case '[':
      gTextStuff.incTextureSize(false);
      break;
   case ']':
      gTextStuff.incTextureSize(true);
      break;

   case '<':
      gTextStuff.incMaxExtent(false);
      break;
   case '>':
      gTextStuff.incMaxExtent(true);
      break;

   case ',':
      gTextStuff.incLineSpacing(false);
      break;
   case '.':
      gTextStuff.incLineSpacing(true);
      break;

   case '{':
      gTextStuff.incGeoScale(false);
      break;
   case '}':
      gTextStuff.incGeoScale(true);
      break;


   case 'f':
      gTextStuff.goToNextFamily();
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
