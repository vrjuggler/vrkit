// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_BASIC_HIGHLIGHTER_H_
#define _INF_BASIC_HIGHLIGHTER_H_

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGColor.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/ViewerPtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/GeometryHighlightTraverser.h>
#include <IOV/Widget/Event.h>
#include <IOV/Util/BasicHighlighterPtr.h>


namespace inf
{

/**
 * Object highlighter that responds to the signals defined in inf::EventData.
 * User applications must instantiate and initialize an object of this type
 * in order for object highlighting to be available in response to object
 * intersection and selection events. Object highlighting is done using
 * inf::GeometryHighlightTraverser.
 *
 * @since 0.19.0
 */
class BasicHighlighter
   : public boost::enable_shared_from_this<BasicHighlighter>
{
public:
   static BasicHighlighterPtr create()
   {
      return BasicHighlighterPtr(new BasicHighlighter());
   }

   ~BasicHighlighter();

   /**
    * Initializes and configures this highlighter. If the configuration
    * associated with the given application object contains a config element
    * that we can handle, then this object is configured based on that config
    * element. Otherwise, built-in defaults are used for the highlighting
    * behavior and appearance.
    *
    * @post This highlighter is configured and ready to be used. Its slots
    *       are connected to the object intersection and selection signals
    *       defined in inf::EventData.
    *
    * @param viewer The VR Juggler application object with which this
    *               highlighter will be used.
    *
    * @see getElementType()
    */
   void init(inf::ViewerPtr viewer);

   /**
    * Responds to object intersection signals. If \p obj is newly intersected,
    * then our intersection highlight is applied to it. Otherwise, our
    * intersection highlight is removed.
    *
    * @post If \p intersected is true, then our intersection highlight is
    *       applied to \p obj. Otherwise, \p obj has no highlights from us.
    *
    * @param obj         The object associated with the intersection signal.
    * @param intersected Indicates whether \p obj is newly intersected or was
    *                    intersected and is no longer.
    */
   inf::Event::ResultType objectIntersected(inf::SceneObjectPtr obj,
                                            const bool intersected);

   /**
    * Responds to object selection signals. If \p obj is newly selected, then
    * our selection highlight is applied to it. Otherwise, our selection
    * highlihght is removed and replaced by our intersection highlight.
    *
    * @pre \p obj has been intersected and objectIntersected() has been called
    *      accordingly.
    * @post If \p selected is true, then our selection highlight is applied to
    *       \p obj. Otherwise, \p our selection highlight is replaced by our
    *       intersection highlight.
    *
    * @param obj      The object associated with the selection signal.
    * @param selected Indicates whether \p obj is newly selected or was
    *                 selected and is no longer.
    */
   inf::Event::ResultType objectSelected(inf::SceneObjectPtr obj,
                                         const bool selected);

protected:
   BasicHighlighter();

private:
   static std::string getElementType()
   {
      return "basic_highlighter";
   }

   /**
    * @throw inf::Exception is thrown if configuration fails.
    */
   void configure(jccl::ConfigElementPtr cfgElt);

   std::vector<boost::filesystem::path> mShaderSearchPath;

   /** @name Highlight Properties */
   //@{
   bool         mEnableShaders;
   std::string  mIsectVertexShaderFile;
   std::string  mIsectFragmentShaderFile;
   OSG::Color3f mIntersectColor;
   float        mIsectUniformScale;
   float        mIsectUniformExponent;
   std::string  mGrabVertexShaderFile;
   std::string  mGrabFragmentShaderFile;
   OSG::Color3f mGrabColor;
   float        mGrabUniformScale;
   float        mGrabUniformExponent;
   //@}

   /** @name Geometry Traverser Properties */
   //@{
   inf::GeometryHighlightTraverser mGeomTraverser;
   unsigned int mIsectHighlightID;
   unsigned int mGrabHighlightID;
   //@}

   boost::signals::connection mIsectConnection;
   boost::signals::connection mDeIsectConnection;
   boost::signals::connection mSelectConnection;
   boost::signals::connection mDeselectConnection;
};

}


#endif /* _INF_BASIC_HIGHLIGHTER_H_ */
