#ifndef SCENE_H
#define SCENE_H

#include <OpenSG/VRJ/Viewer/IOV/ScenePtr.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGRefPtr.h>
#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGTransform.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/IOV/SceneData.h>

#include <vpr/Util/GUID.h>

namespace inf
{

   typedef OSG::CoredNodePtr<OSG::Group>       CoredGroupPtr;
   typedef OSG::CoredNodePtr<OSG::Transform>   CoredTransformPtr;

/**
 * Class to store the data for the scene.
 *
 * This is the main holder for all data that is part of the "model" pattern in
 * the application.  Because of this it is also that place where plugins
 * and the application share data.
 *
 * Coordinate systems used:
 *
 * - V - Virtual World (vw): This is the coord sys of the objects in the scene
 * - P - Platform (plat): This is the platform moving through the virtual world.
 * - W - World (rw): This is the coord sys of the realworld of the user.
 *
 */
class Scene : public boost::enable_shared_from_this<Scene>
{
public:
   /** Factory method for scenes. */
   static ScenePtr create();

   virtual ~Scene()
   {;}

   virtual void init();

   /** Getter/Setters for the scene data. */
   //@{
   CoredGroupPtr getSceneRoot()
   {
      return mSceneRoot;
   }
   void setSceneRoot(CoredGroupPtr scene)
   {
      mSceneRoot = scene;
   }

   CoredGroupPtr getDecoratorRoot()
   {
      return mDecoratorRoot;
   }
   void setDecoratorRoot(CoredGroupPtr root)
   {
      mDecoratorRoot = root;
   }

   CoredTransformPtr getTransformRoot()
   {
      return mTransformRoot;
   }
   void setTransformRoot(CoredTransformPtr root)
   {
      mTransformRoot = root;
   }
   //@}

   /** @name Accessors for Scene Data */
   //@{
   /**
    * Sets the value for the given key in the collection of scene data.  If
    * a value is already associated with \c key, then that value is replaced
    * with \c value.  The previous value is returned to the caller.
    *
    * @param key        the key (index) used for storing the given value
    * @param value      the value to store in the collection of scene data
    *
    * @returns A \c inf::SceneDataPtr is returned to the caller.  If the
    *          collection of scene data already had a value associated with
    *          \c key, then the old value is returned.  Otherwise, the
    *          returned object is a NULL shared pointer.
    */
   inf::SceneDataPtr setSceneData(const vpr::GUID& key,
                                  inf::SceneDataPtr value);

   /**
    * Returns the value in the collection of scene data associated with the
    * given key.
    *
    * @param key        the key (index) of the value to be returned
    *
    * @returns A \c inf::SceneDataPtr is returned to the caller.  If the
    *          collection of scene data has a value associated with \c key,
    *          then that value is returned.  Otherwise, the returned object
    *          is a NULL shared pointer.
    */
   inf::SceneDataPtr getSceneData(const vpr::GUID& key) const;

   /**
    * Removes the value in the container of scene data associated with the
    * given key.  The removed value is returned to the caller.  If there is
    * no value associated with the given key, then this method has no effect.
    *
    * @param key        the key (index) of the value to be removed
    *
    * @returns A \c inf::SceneDataPtr is returned to the caller.  If the
    *          collection of scene data has a value associated with \c key,
    *          then the old value is returned.  Otherwise, the returned object
    *          is a NULL shared pointer.
    */
   inf::SceneDataPtr removeSceneData(const vpr::GUID& key);
   //@}

protected:
   Scene()
   {;}

protected:
   //                  SceneRoot::Group
   //                   /        |
   //  DecoratorRoot:Group      TransformRoot:TransformCore
   //       |                    |
   //       rw                   vw
   //
   // So DecoratorRoot is in the rw coordinate system and
   //    Everything under the TransformCore is in the virtual world coordinate system
   //
   CoredGroupPtr        mSceneRoot;
   CoredGroupPtr        mDecoratorRoot;
   CoredTransformPtr    mTransformRoot;

   //std::map<std::string, OSG::NodePtr>  scene_or_model_nodes;  // ???

private:
   /** @link association */
   /*# SceneData lnkSceneData; */
   std::map<vpr::GUID, SceneDataPtr>   mSceneData;
};

}

#endif
