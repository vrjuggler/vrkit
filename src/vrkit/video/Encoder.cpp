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

#include <vrkit/video/Encoder.h>


namespace vrkit
{

namespace video
{

Encoder::Encoder()
{
   /* Do nothing. */ ;
}

Encoder::~Encoder()
{
   /* Do nothing. */ ;
}

const Encoder::container_format_list_t&
Encoder::getSupportedContainersAndCodecs() const
{
   return mContainerFormatInfoList;
}

void Encoder::setEncodingParameters(const EncoderParameters& params)
{
   mEncoderParams = params;
}

OSG::Image::PixelFormat Encoder::getPixelFormat() const
{
   return OSG::Image::OSG_RGB_PF;
}

const std::string& Encoder::getContainerFormat() const
{
   return mEncoderParams.mContainerFormat;
}

const std::string& Encoder::getCodec() const
{
   return mEncoderParams.mCodec;
}

vpr::Uint32 Encoder::getWidth() const
{
   return mEncoderParams.mWidth;
}

vpr::Uint32 Encoder::getHeight() const
{
   return mEncoderParams.mHeight;
}

const std::string& Encoder::getFilename() const
{
   return mEncoderParams.mFilename;
}

vpr::Uint32 Encoder::getFramesPerSecond() const
{
   return mEncoderParams.mFramesPerSecond;
}

}

}
