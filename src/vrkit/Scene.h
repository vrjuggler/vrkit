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

#ifndef _VRKIT_SCENE_H_
#define _VRKIT_SCENE_H_

#include <vrkit/Config.h>

#include <boost/concept_check.hpp>
#include <boost/static_assert.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGRefPtr.h>
#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGTransform.h>

#include <vpr/Util/GUID.h>

#include <vrkit/SceneData.h>
#include <vrkit/ScenePtr.h>


namespace vrkit
{

   typedef OSG::GroupNodePtr     CoredGroupPtr;
   typedef OSG::TransformNodePtr CoredTransformPtr;

/** \class Scene Scene.h vrkit/Scene.h
 *
 * Class to store the data for the scene.
 *
 * This is the main holder for all data that is part of the "model" pattern in
 * the application.  Because of this it is also that place where plug-ins
 * and the application share data.
 *
 * Coordinate systems used:
 *
 * - V - Virtual World (vw): This is the coord sys of the objects in the scene
 * - P - Platform (plat): This is the platform moving through the virtual world.
 * - W - World (rw): This is the coord sys of the realworld of the user.
 *
 */
class VRKIT_CLASS_API Scene : public boost::enable_shared_from_this<Scene>
{
protected:
   Scene()
   {;}

public:
   /** Factory method for scenes. */
   static ScenePtr create();

   virtual ~Scene();

   /**
    * Initializes the scene data structures.
    *
    * @post All OpenSG scene data structures for this object will be setup.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual ScenePtr init();

   /** Getter/Setters for the scene data. */
   //@{
   OSG::GroupNodePtr getSceneRoot()
   {
      return mSceneRoot;
   }
   void setSceneRoot(OSG::GroupNodePtr scene)
   {
      mSceneRoot = scene;
   }

   OSG::GroupNodePtr getDecoratorRoot()
   {
      return mDecoratorRoot;
   }
   void setDecoratorRoot(OSG::GroupNodePtr root)
   {
      mDecoratorRoot = root;
   }

   OSG::TransformNodePtr getTransformRoot()
   {
      return mTransformRoot;
   }
   void setTransformRoot(OSG::TransformNodePtr root)
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
    * @returns A \c vrkit::SceneDataPtr is returned to the caller.  If the
    *          collection of scene data already had a value associated with
    *          \c key, then the old value is returned.  Otherwise, the
    *          returned object is a NULL shared pointer.
    */
   SceneDataPtr setSceneData(const vpr::GUID& key, SceneDataPtr value);

   /**
    * Returns the value in the collection of scene data associated with the
    * given key.  If no value is associated with the given key, then a value
    * is creaetd using \c T::create() (which means that \c T must have a
    * static method \c create() that returns \c boost::shared_ptr<T>).
    *
    * @post If no value is already associated with \c key, then a value of
    *       type \c boost::shared_ptr<T> is created and stored in the
    *       collection of scene data.
    *
    * @param key        the key (index) of the value to be returned
    *
    * @returns A shared pointer of type \c boost::shared_ptr<T> is returned to
    *          the caller that is guaranteed to be valid.  If the collection
    *          of scene data has a value associated with \c key, then that
    *          value is returned.  Otherwise, a new object is created and
    *          returned.
    */
   template<typename T>
   boost::shared_ptr<T> getSceneData(const vpr::GUID& key)
   {
      // Be sure that the use of boost::dynamic_pointer_cast will succeed.
      BOOST_STATIC_ASSERT((boost::is_convertible<T, SceneData>::value));

      boost::shared_ptr<T> value;

      std::map<vpr::GUID, SceneDataPtr>::iterator i = mSceneData.find(key);

      if ( i == mSceneData.end() )
      {
         value = T::create();
         setSceneData(key, value);
      }
      else
      {
         SceneDataPtr base_value = (*i).second;
         value = boost::dynamic_pointer_cast<T>(base_value);
      }

      return value;
   }

   /**
    * Returns the value in the collection of scene data associated with the
    * given key.  If no value is associated with the given key, then a value
    * is creaetd using \c T::create() (which means that \c T must have a
    * static method \c create() that returns \c boost::shared_ptr<T>).
    * Furthermore, \c T must have a static data member \c type_guid that
    * provides the key (index) of the value to be returned.
    *
    * @post If no value is already associated with \c T::type_guid, then a
    *       value of type \c boost::shared_ptr<T> is created and stored in the
    *       collection of scene data.
    *
    * @returns A shared pointer of type \c boost::shared_ptr<T> is returned to
    *          the caller that is guaranteed to be valid.  If the collection
    *          of scene data has a value associated with \c T::type_guid, then
    *          that value is returned.  Otherwise, a new object is created and
    *          returned.
    *
    * @since 0.14.1
    */
   template<typename T>
   boost::shared_ptr<T> getSceneData()
   {
      return getSceneData<T>(T::type_guid);
   }

   /**
    * Removes the value in the container of scene data associated with the
    * given key.  The removed value is returned to the caller.  If there is
    * no value associated with the given key, then this method has no effect.
    *
    * @param key        the key (index) of the value to be removed
    *
    * @returns A vrkit::SceneDataPtr is returned to the caller.  If the
    *          collection of scene data has a value associated with \c key,
    *          then the old value is returned.  Otherwise, the returned object
    *          is a NULL shared pointer.
    */
   SceneDataPtr removeSceneData(const vpr::GUID& key);
   //@}

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
   OSG::GroupNodePtr        mSceneRoot;
   OSG::GroupNodePtr        mDecoratorRoot;
   OSG::TransformNodePtr    mTransformRoot;

   //std::map<std::string, OSG::NodePtr>  scene_or_model_nodes;  // ???

private:
   /** @link association */
   /*# SceneData lnkSceneData; */
   std::map<vpr::GUID, SceneDataPtr>   mSceneData;
};

}


#endif /* _VRKIT_SCENE_H_ */
