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

#ifndef _VRKIT_MATERIAL_POOL_DATA_H_
#define _VRKIT_MATERIAL_POOL_DATA_H_

#include <vrkit/Config.h>

#include <OpenSG/OSGConfig.h>

#if OSG_MAJOR_VERSION < 2
#  include <OpenSG/OSGMaterialPool.h>
#else
#  include <OpenSG/OSGContainerPool.h>
#endif

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <vrkit/SceneData.h>
#include <vrkit/scenedata/MaterialPoolDataPtr.h>


namespace vrkit
{

/** \class MaterialPoolData MaterialPoolData.h vrkit/scenedata/MaterialPoolData.h
 *
 * Provides a central store for a single instance of OSG::MaterialPool for
 * OpenSG 1.8 or OSG::ContainerPool for OpenSG 2.0. This is used by the
 * Material Chooser Plug-in. Code wishing to register materials for use by
 * that plug-in must do so through this class.
 *
 * @since 0.48.2
 */
class VRKIT_CLASS_API MaterialPoolData : public SceneData
{
private:
   MaterialPoolData();

public:
   /**
    * @name Material Pool Types
    *
    * OpenSG 1.8/2.0 compatibility typedefs. These are mainly for internal
    * use, but they can be used in user-level code to help bridge the
    * differences between OpenSG 1.8 and 2.0.
    *
    * @see getMaterialPool()
    *
    * @since 0.51.0
    */
   //@{
#if OSG_MAJOR_VERSION < 2
   typedef OSG::MaterialPool       pool_t;
   typedef OSG::MaterialPool::Ptr  pool_ptr_t;
#else
   typedef OSG::ContainerPool      pool_t;
   typedef OSG::ContainerPoolPtr   pool_ptr_t;
#endif
   typedef OSG::RefPtr<pool_ptr_t> pool_ref_ptr_t;
   //@}

   static const vpr::GUID type_guid;

   static MaterialPoolDataPtr create()
   {
      return MaterialPoolDataPtr(new MaterialPoolData());
   }

   virtual ~MaterialPoolData();

   pool_ref_ptr_t getMaterialPool();

private:
   pool_ref_ptr_t mMaterialPool;
};

}


#endif /* _VRKIT_MATERIAL_POOL_DATA_H_ */
