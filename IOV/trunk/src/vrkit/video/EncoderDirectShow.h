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

#ifdef VRKIT_WITH_DIRECT_SHOW

#ifndef _VRKIT_VIDEO_ENCODER_DIRECT_SHOW_H_
#define _VRKIT_VIDEO_ENCODER_DIRECT_SHOW_H_

#include <vrkit/Config.h>

#include <atlbase.h>
#include <strmif.h>     /* For IBaseFilter */
#include <control.h>    /* For IMediaControl */
#include <string>
#include <vector>
#include <vpr/vprTypes.h>

#include <vrkit/Video/Encoder.h>

// Register the graph so that we can look at it in graphedt.
#define REGISTER_GRAPH


namespace vrkit
{

namespace video
{

class ByteSource;
class ByteStream;

/**
 * Video encoder implementation for DirectShow. This class provides the link
 * between OpenGL and DirectShow so that the OpenGL frame buffer can be used
 * as input for a DirectShow filter graph.
 *
 * @note This class was renamed from vrkit::DirectShowEncoder and moved into
 *       the vrkit::video namespace in version 0.47.
 *
 * @since 0.44
 */
class EncoderDirectShow : public Encoder
{
public:
   typedef std::vector< CComPtr<IMoniker> > moniker_list_t;

protected:
   EncoderDirectShow();

public:
   static EncoderPtr create();

   /**
    * Destructor closes the movie file and flushes all the frames
    */
   ~EncoderDirectShow();

   /**
    * Takes care of creating the memory, streams, compression options etc.
    * required for the movie.
    */
   virtual EncoderPtr init();

   /** @name Encoding interface. */
   //@{
   virtual void startEncoding();

   /**
    * Takes care of releasing the memory and movie related handles
    */
   virtual void stopEncoding();

   /**
    * Inserts the given bitmap bits into the movie as a new Frame at the end.
    * The width, height and bitsPerPixel are the width, height and bits per
    * pixel of the bitmap pointed to by the input pBits.
    */
   virtual void writeFrame(vpr::Uint8* data);

   virtual OSG::Image::PixelFormat getPixelFormat()
   {
      return OSG::Image::OSG_BGR_PF;
   }
   //@}

   /**
    * Return a list of valid codecs.
    */
   static codec_list_t getCodecs();

   static std::string getName()
   {
      return "DirectShow Encoder";
   }

   static void printVideoEncoders();

protected:
   static moniker_list_t getVideoMonikers();
   static CComPtr<IBaseFilter> getEncoder(const std::string& name);
   static void printPins(CComPtr<IBaseFilter> filter);

   void setAviOptions(CComPtr<IBaseFilter> filter,
                      const InterleavingMode mode);

private:
   CComPtr<IGraphBuilder> mGraphBuilder;
   CComPtr<IMediaControl> mMediaController;
   ByteSource* mByteSource;
   ByteStream* mByteStream;
   bool        mRunning;
#ifdef REGISTER_GRAPH
   DWORD mGraphRegister;
#endif
};

}

}


#endif /* _VRKIT_VIDEO_ENCODER_DIRECT_SHOW_H_ */

#endif /* VRKIT_WITH_DIRECT_SHOW */
