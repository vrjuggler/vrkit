// vrkit is (C) Copyright 2005-2008
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

#include <vrkit/scenedata/MaterialPoolData.h>


namespace vrkit
{

const vpr::GUID
MaterialPoolData::type_guid("3f933047-ef3d-44fb-9553-d6262fcc000b");

MaterialPoolData::MaterialPoolData()
   : mMaterialPool(OSG::NullFC)
{
   /* Do nothing. */ ;
}

MaterialPoolData::~MaterialPoolData()
{
   /* Do nothing. */ ;
}

MaterialPoolData::pool_ref_ptr_t MaterialPoolData::getMaterialPool()
{
   if ( OSG::NullFC == mMaterialPool )
   {
      mMaterialPool = pool_t::create();
   }

   return mMaterialPool;
}

}
