// vrkit is (C) Copyright 2005-2007
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

#include <OpenSG/OSGMaterialPool.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <vrkit/SceneData.h>
#include <vrkit/scenedata/MaterialPoolDataPtr.h>


namespace vrkit
{

/**
 * Provides a central store for a single instance of OSG::MaterialPool. This
 * is used by the Material Chooser Plug-in. Code wishing to register materials
 * for use by the that plug-in must do so through this class.
 *
 * @since 0.48.2
 */
class VRKIT_CLASS_API MaterialPoolData : public SceneData
{
private:
   MaterialPoolData();

public:
   static const vpr::GUID type_guid;

   static MaterialPoolDataPtr create()
   {
      return MaterialPoolDataPtr(new MaterialPoolData());
   }

   virtual ~MaterialPoolData();

   OSG::MaterialPoolRefPtr getMaterialPool();

private:
   OSG::MaterialPoolRefPtr mMaterialPool;
};

}


#endif /* _VRKIT_MATERIAL_POOL_DATA_H_ */
