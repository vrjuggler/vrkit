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

#ifdef VRKIT_WITH_VFW
#pragma once

#ifndef _VRKIT_VIDEO_ENCODER_VFW_H_
#define _VRKIT_VIDEO_ENCODER_VFW_H_

#include <windows.h>
#include <vfw.h>
#include <TChar.h>

#include <vrkit/Config.h>

#include <vpr/vprTypes.h>

#include <vrkit/video/Encoder.h>


namespace vrkit
{

namespace video
{

/** \class EncoderVFW EncoderVFW.h vrkit/video/EncoderVFW.h
 *
 * This is the video encoder implementation based on Video for Windows.
 *
 * @note This class was renamed from vrkit::VfwEncoder and moved into the
 *       vrkit::video namespace in version 0.47.
 */
class EncoderVFW : public Encoder
{
public:
   static EncoderPtr create();

   /**
    * Takes care of creating the memory, streams, compression options etc.
    * required for the movie.
    */
   virtual EncoderPtr init();

   /**
    * Sets the Error Message
    */
   void SetErrorMessage(LPCTSTR lpszErrMsg);

protected:
   /**
    * Constructor accepts the filename, video code and frame rate settings
    * as parameters.
    * lpszFileName: Name of the output movie file to create
    * dwCodec: FourCC of the Video Codec to be used for compression
    * dwFrameRate: The Frames Per Second (FPS) setting to be used for the movie
    * The default Codec used here is MPG4. To use a different codec, pass its Fourcc
    * value as the input parameter for dwCodec.
    * For example,
    * pass mmioFOURCC('D','I','V','X') to use DIVX codec, or
    * pass mmioFOURCC('I','V','5','0') to use IV50 codec etc...
    *
    * Also, you can pass just 0 to avoid the codecs altogether. In that case, Frames
    * would be saved as they are without any compression; However, the output movie file
    * size would be very huge in that case.
    *
    * Finally, make sure you have the codec installed on the machine before
    * passing its Fourcc here.
    */
   EncoderVFW();

public:
   /**
    * Destructor closes the movie file and flushes all the frames
    */
   virtual ~EncoderVFW();

   /** @name Encoding interface. */
   //@{
   virtual void startEncoding();

   /**
    * Takes care of releasing the memory and movie related handles
    */
   virtual void stopEncoding();

   /**
    * Inserts the given bitmap bits into the movie as a new Frame at the end.
    * The width, height and bitsPerPixel are the width, height and bits per pixel
    * of the bitmap pointed to by the input pBits.
    */
   virtual void writeFrame(const vpr::Uint8* data);

   virtual OSG::Image::PixelFormat getPixelFormat()
   {
      return OSG::Image::OSG_BGR_PF;
   }
   //@}

   /**
    * Returns the last error message, if any.
    */
   LPCTSTR GetLastErrorMessage() const
   {
      return mErrorMsg;
   }

   static void printCodecs();

   static codec_list_t getCodecs();

   static std::string getName()
   {
      return "Video For Windows Encoder";
   }

private:
   HDC                  mAviDC;
   vpr::Uint64          mFrameCount;	// Keeps track of the current Frame Index
   PAVIFILE             mAviFile;
   PAVISTREAM           mVideoStream;
   PAVISTREAM           mCompressedVideoStream;
   AVICOMPRESSOPTIONS   mCompressOptions;
   AVISTREAMINFO        mStreamInfo;
   vpr::Uint32          mFrameRate;             /**< Frames Per Second Rate (FPS) */
   vpr::Uint32          mCodecId;         // Video Codec FourCC
   TCHAR                mErrorMsg[MAX_PATH];   // Holds the Last Error Message, if any
};

}

}


#endif /* _VRKIT_VIDEO_ENCODER_VFW_H_ */

#endif /* VRKIT_WITH_VFW */
