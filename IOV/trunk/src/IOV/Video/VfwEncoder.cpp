#ifdef IOV_WITH_VFW

#include <sstream>
#include <IOV/Video/VfwEncoder.h>

#ifndef __countof
#define __countof(x)	((sizeof(x)/sizeof(x[0])))
#endif

#pragma comment(lib, "Vfw32.lib")

namespace inf
{

VfwEncoder::VfwEncoder()
   : mVideoStream(NULL)
   , mCompressedVideoStream(NULL)
   , mFrameCount(0)
   , mAviFile(NULL)
   , mAviDC(NULL)
{
}

EncoderPtr VfwEncoder::create()
{
   return VfwEncoderPtr(new VfwEncoder);
}

void buildAndThrowAviError(const vpr::Uint32 errCode)
{
   switch (errCode)
   {
      case AVIERR_OK:
         return;
      case AVIERR_UNSUPPORTED:
         throw std::exception("AVIERR_UNSUPPORTED");
      case AVIERR_BADFORMAT:
         throw std::exception("AVIERR_BADFORMAT");
      case AVIERR_MEMORY:
         throw std::exception("AVIERR_MEMORY");
      case AVIERR_INTERNAL:
         throw std::exception("AVIERR_INTERNAL");
      case AVIERR_BADFLAGS:
         throw std::exception("AVIERR_BADFLAGS");
      case AVIERR_BADPARAM:
         throw std::exception("AVIERR_BADPARAM");
      case AVIERR_BADSIZE:
         throw std::exception("AVIERR_BADSIZE");
      case AVIERR_BADHANDLE:
         throw std::exception("AVIERR_BADHANDLE");
      case AVIERR_FILEREAD:
         throw std::exception("AVIERR_FILEREAD");
      case AVIERR_FILEWRITE:
         throw std::exception("AVIERR_FILEWRITE");
      case AVIERR_FILEOPEN:
         throw std::exception("AVIERR_FILEOPEN");
      case AVIERR_COMPRESSOR:
         throw std::exception("AVIERR_COMPRESSOR");
      case AVIERR_NOCOMPRESSOR:
         throw std::exception("AVIERR_NOCOMPRESSOR");
      case AVIERR_READONLY:
         throw std::exception("AVIERR_READONLY");
      case AVIERR_NODATA:
         throw std::exception("AVIERR_NODATA");
      case AVIERR_BUFFERTOOSMALL:
         throw std::exception("AVIERR_BUFFERTOOSMALL");
      case AVIERR_CANTCOMPRESS:
         throw std::exception("AVIERR_CANTCOMPRESS");
      case AVIERR_USERABORT:
         throw std::exception("AVIERR_USERABORT");
      case AVIERR_ERROR:
         throw std::exception("AVIERR_ERROR");
      default:
         throw std::exception("AVIERR_UNKOWN_ERROR");
   }
}

EncoderPtr VfwEncoder::init(const std::string& filename, const std::string& codec,
                            const vpr::Uint32 width, const vpr::Uint32 height,
                            const vpr::Uint32 framesPerSecond)
{

   const vpr::Uint32 bitsPerPixel = 24;
   AVIFileInit();
   mFilename = filename;
   int max_width = GetSystemMetrics(SM_CXSCREEN);
   int max_height = GetSystemMetrics(SM_CYSCREEN);
   //mCodecId = mmioFOURCC('M','P','G','2');
   //mCodecId = mmioFOURCC('X', 'V', 'I', 'D');
   //mCodecId = mmioFOURCC('M','P','G','4');
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

   if(width > max_width)
   {
      max_width = width;
   }
   if(height > max_height)
   {
      max_height = height;
   }

   if(FAILED(AVIFileOpen(&mAviFile, mFilename.c_str(), OF_CREATE|OF_WRITE, NULL)))
   {
      std::stringstream err_msg;
      err_msg << "Unable to create movie file: " << mFilename;
      throw std::exception(err_msg.str().c_str());
   }

   ZeroMemory(&mStreamInfo,sizeof(AVISTREAMINFO));
   mStreamInfo.fccType = streamtypeVIDEO;
   mStreamInfo.fccHandler = mCodecId;
   mStreamInfo.dwScale = 1;
   mStreamInfo.dwRate = framesPerSecond; // Frames Per Second;
   mStreamInfo.dwQuality = -1;				// Default Quality
   mStreamInfo.dwSuggestedBufferSize = max_width*max_height*4;
   SetRect(&mStreamInfo.rcFrame, 0, 0, width, height);
   _tcscpy(mStreamInfo.szName, _T("Video Stream"));

   if(FAILED(AVIFileCreateStream(mAviFile,&mVideoStream,&mStreamInfo)))
   {
      throw std::exception("Unable to create video stream in movie file.");
   }

   ZeroMemory(&mCompressOptions,sizeof(AVICOMPRESSOPTIONS));
   mCompressOptions.fccType=streamtypeVIDEO;
   mCompressOptions.fccHandler=mStreamInfo.fccHandler;
   mCompressOptions.dwFlags=AVICOMPRESSF_KEYFRAMES|AVICOMPRESSF_VALID;//|AVICOMPRESSF_DATARATE;
   mCompressOptions.dwKeyFrameEvery=1;
   //mCompressOptions.dwBytesPerSecond=1000/8;
   //mCompressOptions.dwQuality=100;

   vpr::Int32 cres = AVIMakeCompressedStream(&mCompressedVideoStream,mVideoStream,&mCompressOptions,NULL);
   if (AVIERR_NOCOMPRESSOR == cres)
   {
      throw std::exception("A suitable compressor cannot be found.");
   }
   else if (AVIERR_MEMORY == cres)
   {
      throw std::exception("There is not enough memory to complete the operation.");
   }
   else if (AVIERR_UNSUPPORTED == cres)
   {
      // This error might be returned if you try to compress data that is not audio or video.
      throw std::exception("Compression is not supported for this type of data.");
   }
   else if (AVIERR_OK != cres)
   {
      std::cout << "CRES: " << cres << std::endl;
      buildAndThrowAviError(cres);
      // One reason this error might occur is if you are using a Codec that is not 
      // available on your system. Check the mmioFOURCC() code you are using and make
      // sure you have that codec installed properly on your machine.
      throw std::exception("Unable to create compressed stream: Check your codec options.");
   }

   mWidth = width;
   mHeight = height;

   BITMAPINFO bmpInfo;
   ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
   bmpInfo.bmiHeader.biPlanes = 1;
   bmpInfo.bmiHeader.biWidth = width;
   bmpInfo.bmiHeader.biHeight = height;
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

   return shared_from_this();
}


VfwEncoder::~VfwEncoder()
{
   close();
}

void VfwEncoder::close()
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

void VfwEncoder::SetErrorMessage(LPCTSTR lpszErrorMessage)
{
   _tcsncpy(mErrorMsg, lpszErrorMessage, __countof(mErrorMsg)-1);
}
	

void VfwEncoder::writeFrame(int width, int height, vpr::Uint8* data)
{
   const int bitsPerPixel=24;
   vpr::Uint32 dwSize = width * height * (bitsPerPixel/8);

   if(FAILED(AVIStreamWrite(mCompressedVideoStream,mFrameCount++,1,data,dwSize,0,NULL,NULL)))
   {
      SetErrorMessage(_T("Unable to Write Video Stream to the output Movie File"));
      close();
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

VfwEncoder::codec_list_t VfwEncoder::getCodecs()
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

void VfwEncoder::printCodecs()
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

#endif /*IOV_WITH_VFW*/
