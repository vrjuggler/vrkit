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

#ifndef _VRKIT_VIDEO_ENCODER_H_
#define _VRKIT_VIDEO_ENCODER_H_

#include <vrkit/Config.h>

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include <OpenSG/OSGImage.h>

#include <vpr/vprTypes.h>
#include <vpr/Util/Assert.h>

#include <vrkit/video/EncoderPtr.h>


namespace vrkit
{

namespace video
{

/** \class Encoder Encoder.h vrkit/video/Encoder.h
 *
 * This is the abstract base class for all video encoder implementations.
 *
 * @note This class was moved into the vrkit::video namespace in version 0.47.
 */
class VRKIT_CLASS_API Encoder
   : public boost::enable_shared_from_this<Encoder>
   , private boost::noncopyable
{
protected:
   Encoder();

public:
   virtual ~Encoder();

   /** @name Encoder Data Types */
   //@{
   typedef std::vector<std::string> codec_list_t;

   /**
    * @since 0.50.0
    *
    * @note This was renamed from container_format_info_t in version 0.50.0
    */
   struct ContainerFormatInfo
   {
      std::string               mFormatName;
      std::string               mFormatLongName;
      std::vector<std::string>  mFileExtensions;
      codec_list_t              mCodecList;
      std::string               mEncoderName;
   };

   /**
    * @since 0.50.0
    *
    * @note This was renamed from encoder_parameters_t in version 0.50.0
    */
   struct EncoderParameters
   {
      std::string mContainerFormat;
      std::string mCodec;
      std::string mFilename;
      vpr::Uint32 mWidth;
      vpr::Uint32 mHeight;
      vpr::Uint32 mFramesPerSecond;
   };

   typedef std::vector<ContainerFormatInfo> container_format_list_t;
   //@}

   virtual EncoderPtr init() = 0;

   const container_format_list_t& getSupportedContainersAndCodecs() const;

   void setEncodingParameters(const EncoderParameters& params);

   /**
    * @throw vrkit::RecordingException Thrown if starting recording fails.
    */
   virtual void startEncoding() = 0;

   virtual void stopEncoding() = 0;

   /**
    * @throw vrkit::RecordingException
    *           Thrown if writing the given frame data fails.
    */
   virtual void writeFrame(vpr::Uint8* data) = 0;

   virtual OSG::Image::PixelFormat getPixelFormat() const;

protected:

   /** Helper functions into the encoder param struct. */
   //@{
   const std::string& getContainerFormat() const;
   const std::string& getCodec() const;
   vpr::Uint32 getWidth() const;
   vpr::Uint32 getHeight() const;
   const std::string& getFilename() const;
   vpr::Uint32 getFramesPerSecond() const;
   //}

   EncoderParameters            mEncoderParams;
   container_format_list_t      mContainerFormatInfoList;
};

}

}


#endif /* _VRKIT_VIDEO_ENCODER_H_ */
