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

#ifndef _VRKIT_VIDEO_ENCODER_PTR_H_
#define _VRKIT_VIDEO_ENCODER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace vrkit
{

namespace video
{

   class Encoder;
   typedef boost::shared_ptr<Encoder> EncoderPtr;
   typedef boost::weak_ptr<Encoder> EncoderWeakPtr;

}

}

#endif /* _VRKIT_VIDEO_ENCODER_PTR_H_ */
