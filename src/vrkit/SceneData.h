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

#ifndef _VRKIT_SCENE_DATA_H_
#define _VRKIT_SCENE_DATA_H_

#include <vrkit/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <vrkit/SceneDataPtr.h>


namespace vrkit
{

/** \class SceneData SceneData.h vrkit/SceneData.h
 *
 * The base class for all "scene data" managed by vrkit::Scene.
 */
class VRKIT_CLASS_API SceneData
   : public boost::enable_shared_from_this<SceneData>
{
protected:
   SceneData();

public:
   virtual ~SceneData();
};

}


#endif /* _VRKIT_SCENE_DATA_H_ */
