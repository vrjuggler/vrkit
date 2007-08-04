#ifdef IOV_WITH_VFW
#pragma once

#ifndef _INF_VFW_ENCODER_H_
#define _INF_VFW_ENCODER_H_

#include <windows.h>
#include <vfw.h>
#include <TChar.h>
#include <IOV/Config.h>
#include <vpr/vprTypes.h>

#include <IOV/Video/Encoder.h>
#include <IOV/Video/VfwEncoderPtr.h>

namespace inf
{

class IOV_CLASS_API VfwEncoder : public Encoder
{
public:
   static EncoderPtr create();

   /**
    * Takes care of creating the memory, streams, compression options etc. required for the movie
    */
   virtual EncoderPtr init(const std::string& filename, const std::string& codec,
                           const vpr::Uint32 width, const vpr::Uint32 height,
                           const vpr::Uint32 framesPerSecond);
   /**
    * Takes care of releasing the memory and movie related handles
    */
   virtual void close();

   /**
    * Sets the Error Message
    */
   void SetErrorMessage(LPCTSTR lpszErrMsg);

   virtual vpr::Uint32 width() const
   {
      return mWidth;
   }

   virtual vpr::Uint32 height() const
   {
      return mHeight;
   }

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
   VfwEncoder();

public:
   /**
    * Destructor closes the movie file and flushes all the frames
    */
   ~VfwEncoder();

   /**
    * Inserts the given bitmap bits into the movie as a new Frame at the end.
    * The width, height and bitsPerPixel are the width, height and bits per pixel
    * of the bitmap pointed to by the input pBits.
    */
   void writeFrame(int width, int height, vpr::Uint8* data);

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
      return "VideoForWindowsEncoder";
   }

private:
   HDC                  mAviDC;
   vpr::Uint64          mFrameCount;	// Keeps track of the current Frame Index
   vpr::Uint32          mWidth;
   vpr::Uint32          mHeight;
   PAVIFILE             mAviFile;
   PAVISTREAM           mVideoStream;
   PAVISTREAM           mCompressedVideoStream;
   AVICOMPRESSOPTIONS   mCompressOptions;
   AVISTREAMINFO        mStreamInfo;
   vpr::Uint32          mFrameRate;             /**< Frames Per Second Rate (FPS) */
   vpr::Uint32          mCodecId;         // Video Codec FourCC
   std::string          mFilename; // Holds the Output Movie File Name
   TCHAR                mErrorMsg[MAX_PATH];   // Holds the Last Error Message, if any
};

}

#endif /*_INF_VFW_ENCODER_H_*/

#endif /*IOV_WITH_VFW*/
