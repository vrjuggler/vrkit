// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Video/Encoder.h>


namespace inf
{

Encoder::Encoder()
{
   /* Do nothing. */ ;
}

Encoder::~Encoder()
{
   /* Do nothing. */ ;
}

Encoder::container_format_list_t Encoder::getSupportedContainersAndCodecs() const
{
   return mContainerFormatInfoList;
}

void Encoder::setEncodingParameters(const encoder_parameters_t params)
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
