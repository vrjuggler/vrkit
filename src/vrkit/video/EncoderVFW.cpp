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

#ifdef VRKIT_WITH_VFW

#include <sstream>

#include <vrkit/Exception.h>
#include <vrkit/exceptions/BadRecordingData.h>
#include <vrkit/exceptions/RecordingException.h>
#include <vrkit/exceptions/RecordingConfigError.h>
#include <vrkit/video/EncoderVFW.h>

#ifndef __countof
#define __countof(x)	((sizeof(x)/sizeof(x[0])))
#endif

#pragma comment(lib, "Vfw32.lib")


namespace vrkit
{

namespace video
{

EncoderVFW::EncoderVFW()
   : mVideoStream(NULL)
   , mCompressedVideoStream(NULL)
   , mFrameCount(0)
   , mAviFile(NULL)
   , mAviDC(NULL)
{
}

EncoderPtr EncoderVFW::create()
{
   return EncoderPtr(new EncoderVFW);
}

void buildAndThrowAviError(const vpr::Uint32 errCode)
{
   switch (errCode)
   {
      case AVIERR_OK:
         return;
      case AVIERR_UNSUPPORTED:
         throw Exception("AVIERR_UNSUPPORTED", VRKIT_LOCATION);
      case AVIERR_BADFORMAT:
         throw Exception("AVIERR_BADFORMAT", VRKIT_LOCATION);
      case AVIERR_MEMORY:
         throw Exception("AVIERR_MEMORY", VRKIT_LOCATION);
      case AVIERR_INTERNAL:
         throw Exception("AVIERR_INTERNAL", VRKIT_LOCATION);
      case AVIERR_BADFLAGS:
         throw Exception("AVIERR_BADFLAGS", VRKIT_LOCATION);
      case AVIERR_BADPARAM:
         throw Exception("AVIERR_BADPARAM", VRKIT_LOCATION);
      case AVIERR_BADSIZE:
         throw Exception("AVIERR_BADSIZE", VRKIT_LOCATION);
      case AVIERR_BADHANDLE:
         throw Exception("AVIERR_BADHANDLE", VRKIT_LOCATION);
      case AVIERR_FILEREAD:
         throw Exception("AVIERR_FILEREAD", VRKIT_LOCATION);
      case AVIERR_FILEWRITE:
         throw Exception("AVIERR_FILEWRITE", VRKIT_LOCATION);
      case AVIERR_FILEOPEN:
         throw Exception("AVIERR_FILEOPEN", VRKIT_LOCATION);
      case AVIERR_COMPRESSOR:
         throw Exception("AVIERR_COMPRESSOR", VRKIT_LOCATION);
      case AVIERR_NOCOMPRESSOR:
         throw Exception("AVIERR_NOCOMPRESSOR", VRKIT_LOCATION);
      case AVIERR_READONLY:
         throw Exception("AVIERR_READONLY", VRKIT_LOCATION);
      case AVIERR_NODATA:
         throw Exception("AVIERR_NODATA", VRKIT_LOCATION);
      case AVIERR_BUFFERTOOSMALL:
         throw Exception("AVIERR_BUFFERTOOSMALL", VRKIT_LOCATION);
      case AVIERR_CANTCOMPRESS:
         throw Exception("AVIERR_CANTCOMPRESS", VRKIT_LOCATION);
      case AVIERR_USERABORT:
         throw Exception("AVIERR_USERABORT", VRKIT_LOCATION);
      case AVIERR_ERROR:
         throw Exception("AVIERR_ERROR", VRKIT_LOCATION);
      default:
         throw Exception("AVIERR_UNKOWN_ERROR", VRKIT_LOCATION);
   }
}

EncoderPtr EncoderVFW::init()
{
   ContainerFormatInfo format_info;
   format_info.mFormatName = "AVI";
   format_info.mFormatLongName = "Audio Video Interleave";
   format_info.mFileExtensions.push_back("avi");
   format_info.mCodecList = EncoderVFW::getCodecs();
   format_info.mEncoderName = EncoderVFW::getName();

   mContainerFormatInfoList.push_back(format_info);

   return shared_from_this();
}

void EncoderVFW::startEncoding()
{
   mFrameCount = 0;

   const vpr::Uint32 bitsPerPixel = 24;
   AVIFileInit();
   int max_width = GetSystemMetrics(SM_CXSCREEN);
   int max_height = GetSystemMetrics(SM_CYSCREEN);
   //mCodecId = mmioFOURCC('M','P','G','2');
   //mCodecId = mmioFOURCC('X', 'V', 'I', 'D');
   //mCodecId = mmioFOURCC('M','P','G','4');
   const std::string& codec = getCodec();

   if ( 4 != codec.size() )
   {
      throw RecordingConfigError("Video for Windows codecs must be 4 chars.",
                                 VRKIT_LOCATION);
   }

   mCodecId = mmioFOURCC(codec[0], codec[1], codec[2], codec[3]);
   //to use IV50 codec etc...
   //mCodecId = mmioFOURCC('I','V','5','0');
   //mCodecId = mmioFOURCC('M','S','V','C');
   //mCodecId = mmioFOURCC('W','M','V','3');
   //mCodecId = mmioFOURCC('D','I','V','X');

   mAviDC = CreateCompatibleDC(NULL);
   if(mAviDC==NULL)
   {
      throw std::exception("Unable to create compatible DC");
   }

   if(getWidth() > max_width)
   {
      std::stringstream ss;
      ss << "Can't encode at width: " << getWidth() << " max: " << max_width;
      throw RecordingException(ss.str(), VRKIT_LOCATION);
   }
   if(getHeight() > max_height)
   {
      std::stringstream ss;
      ss << "Can't encode at height: " << getHeight() << " max: " << max_width;
      throw RecordingException(ss.str(), VRKIT_LOCATION);
   }

   if(FAILED(AVIFileOpen(&mAviFile, getFilename().c_str(), OF_CREATE|OF_WRITE, NULL)))
   {
      std::stringstream err_msg;
      err_msg << "Unable to create movie file: " << getFilename();
      throw RecordingException(err_msg.str(), VRKIT_LOCATION);
   }

   ZeroMemory(&mStreamInfo,sizeof(AVISTREAMINFO));
   mStreamInfo.fccType = streamtypeVIDEO;
   mStreamInfo.fccHandler = mCodecId;
   mStreamInfo.dwScale = 1;
   mStreamInfo.dwRate = getFramesPerSecond(); // Frames Per Second;
   mStreamInfo.dwQuality = -1;				// Default Quality
   mStreamInfo.dwSuggestedBufferSize = getWidth() * getHeight() * 3;
   SetRect(&mStreamInfo.rcFrame, 0, 0, getWidth(), getHeight());
   _tcscpy(mStreamInfo.szName, _T("Video Stream"));

   if(FAILED(AVIFileCreateStream(mAviFile,&mVideoStream,&mStreamInfo)))
   {
      throw RecordingException(
         "Unable to create video stream in movie file.", VRKIT_LOCATION
      );
   }

   ZeroMemory(&mCompressOptions,sizeof(AVICOMPRESSOPTIONS));
   mCompressOptions.fccType=streamtypeVIDEO;
   mCompressOptions.fccHandler=mStreamInfo.fccHandler;
   mCompressOptions.dwFlags=AVICOMPRESSF_KEYFRAMES|AVICOMPRESSF_VALID;//|AVICOMPRESSF_DATARATE;
   mCompressOptions.dwKeyFrameEvery=1;
   //mCompressOptions.dwBytesPerSecond=1000/8;
   //mCompressOptions.dwQuality=100;

   const HRESULT cres = AVIMakeCompressedStream(&mCompressedVideoStream,
                                                mVideoStream,
                                                &mCompressOptions, NULL);

   if (AVIERR_NOCOMPRESSOR == cres)
   {
      throw RecordingConfigError("A suitable compressor cannot be found.",
                                 VRKIT_LOCATION);
   }
   else if (AVIERR_MEMORY == cres)
   {
      throw RecordingException(
         "There is not enough memory to complete the operation.",
         VRKIT_LOCATION
      );
   }
   else if (AVIERR_UNSUPPORTED == cres)
   {
      // This error might be returned if you try to compress data that is not audio or video.
      throw BadRecordingData(
         "Compression is not supported for this type of data.", VRKIT_LOCATION
      );
   }
   else if (AVIERR_OK != cres)
   {
      buildAndThrowAviError(cres);
      // One reason this error might occur is if you are using a Codec that is
      // not available on your system. Check the mmioFOURCC() code you are
      // using and make sure you have that codec installed properly on your
      // machine.
      throw RecordingConfigError(
         "Unable to create compressed stream: Check your codec options.",
         VRKIT_LOCATION
      );
   }

   vprASSERT(mCompressedVideoStream != NULL &&
             "We should have a compressed video stream at this point");

   BITMAPINFO bmpInfo;
   ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
   bmpInfo.bmiHeader.biPlanes = 1;
   bmpInfo.bmiHeader.biWidth = getWidth();
   bmpInfo.bmiHeader.biHeight = getHeight();
   bmpInfo.bmiHeader.biCompression = BI_RGB;
   bmpInfo.bmiHeader.biBitCount	= bitsPerPixel;
   bmpInfo.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
   bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biHeight*bmpInfo.bmiHeader.biBitCount/8;

   if(FAILED(AVIStreamSetFormat(mCompressedVideoStream,0,(LPVOID)&bmpInfo, bmpInfo.bmiHeader.biSize)))
   {
      // One reason this error might occur is if your bitmap does not meet the Codec requirements.
      // For example,
      //   your bitmap is 32bpp while the Codec supports only 16 or 24 bpp; Or
      //   your bitmap is 274 * 258 size, while the Codec supports only sizes that are powers of 2; etc...
      // Possible solution to avoid this is: make your bitmap suit the codec requirements,
      // or Choose a codec that is suitable for your bitmap.
      throw std::exception("Unable to set video stream format");
   }
}


EncoderVFW::~EncoderVFW()
{
   stopEncoding();
}

void EncoderVFW::stopEncoding()
{
   if(mAviDC)
   {
      DeleteDC(mAviDC);
      mAviDC=NULL;
   }
   if(mCompressedVideoStream)
   {
      AVIStreamRelease(mCompressedVideoStream);
      mCompressedVideoStream=NULL;
   }
   if(mVideoStream)
   {
      AVIStreamRelease(mVideoStream);
      mVideoStream=NULL;
   }
   if(mAviFile)
   {
      AVIFileRelease(mAviFile);
      mAviFile=NULL;
   }
   AVIFileExit();
}

void EncoderVFW::SetErrorMessage(LPCTSTR lpszErrorMessage)
{
   _tcsncpy(mErrorMsg, lpszErrorMessage, __countof(mErrorMsg)-1);
}

void EncoderVFW::writeFrame(const vpr::Uint8* data)
{
   if ( NULL == mCompressedVideoStream )
   {
      return;
   }

   const int bitsPerPixel=24;
   const vpr::Uint32 size = getWidth() * getHeight() * (bitsPerPixel/8);

   const HRESULT result = AVIStreamWrite(mCompressedVideoStream,
                                         mFrameCount++, 1, data, size, 0,
                                         NULL, NULL);

   if ( FAILED(result) )
   {
      SetErrorMessage(_T("Unable to Write Video Stream to the output Movie File"));
      buildAndThrowAviError(result);
   }
}

std::string dword_to_string(DWORD di)
{
   union
   {
      vpr::Int32 val;
      vpr::Int8  c[5];
   } converter;

   memset(converter.c, '\0', 5);
   converter.val = di;
   return std::string(reinterpret_cast<char*>(converter.c));
}

EncoderVFW::codec_list_t EncoderVFW::getCodecs()
{
   codec_list_t codecs;

   vpr::Int32 info_index = 0;
   vpr::Int32 fccType = ICTYPE_VIDEO;
   ICINFO* info = (ICINFO *)malloc(sizeof(ICINFO));

   bool result = ICInfo(0, info_index, info);
   while (result)
   {
      std::string codec = dword_to_string(info->fccHandler);
      codecs.push_back(codec);
      info_index++;
      result = ICInfo(0, info_index, info);
   }
   delete info;

   return codecs;
}

void EncoderVFW::printCodecs()
{
   vpr::Int32 info_index = 0;
   vpr::Int32 fccType = ICTYPE_VIDEO;
   ICINFO* info = (ICINFO *)malloc(sizeof(ICINFO));

   std::cout << "Installed Codecs" << std::endl;
   std::cout << "================" << std::endl;
   std::cout << "Type Codc Name             Description      Driver" << std::endl;

   bool result = ICInfo(0, info_index, info);
   while (result)
   {
      printf("%4.4s %4.4s %16.16ls %16.16ls %ls\n",
        dword_to_string(info->fccType).c_str(),
        dword_to_string(info->fccHandler).c_str(),
        info->szName, info->szDescription, info->szDriver);

      info_index++;
      result = ICInfo(0, info_index, info);
   }
   delete info;
}

}

}

#endif /* VRKIT_WITH_VFW */
