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

#include <vrkit/SceneData.h>


namespace vrkit
{

// NOTE: This is here in the .cpp file to ensure that vrkit::SceneData is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
SceneData::SceneData()
{
   /* Do nothing. */ ;
}

// NOTE: This is here in the .cpp file to ensure that vrkit::SceneData is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
SceneData::~SceneData()
{
   /* Do nothing. */ ;
}

}
