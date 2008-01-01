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

#ifdef VRKIT_WITH_DIRECT_SHOW

#include <algorithm>

#include <vpr/Util/Assert.h>

#if defined(VRKIT_DEBUG) && defined(_DEBUG)
#   define DS_LIB_RT_OPT "d"
#   define CMT_RT_OPT "d"
#else
#   define DS_LIB_RT_OPT "e"
#   define CMT_RT_OPT ""
#endif

#pragma comment(lib, "strmbas" DS_LIB_RT_OPT ".lib")
#pragma comment(linker, "/nodefaultlib:libcmt" CMT_RT_OPT)

#undef DS_LIB_RT_OPT

#include <vrkit/exceptions/BadRecordingData.h>
#include <vrkit/video/DirectShowSource.h>

// These have to be included after DirectShowSource.h to avoid interfering
// with other uses of DEFINE_GUID() but before our use of DEFINE_GUID() to
// ensure that CLSID_ByteSource is initialized.
#include <ObjBase.h>
#include <InitGuid.h>

// {930C2C1F-A305-44fc-8D1C-99815D89D7C8}
DEFINE_GUID(CLSID_ByteSource, 
0x930c2c1f, 0xa305, 0x44fc, 0x8d, 0x1c, 0x99, 0x81, 0x5d, 0x89, 0xd7, 0xc8);

namespace vrkit
{

namespace video
{

ByteSource* ByteSource::create(LPUNKNOWN owner)
{
   return new ByteSource(owner);
}

ByteSource::ByteSource(LPUNKNOWN owner)
   : CBaseFilter(NAME("ByteSource"), owner, &mLock, CLSID_ByteSource)
   , mByteStreams()
{
   CAutoLock cAutoLock(&mLock);
}

ByteSource::~ByteSource()
{
   for ( stream_list_t::iterator itr = mByteStreams.begin();
         itr != mByteStreams.end(); ++itr )
   {
      vprASSERT(*itr);
      (*itr)->Release();
   }
   mByteStreams.clear();
}

ULONG ByteSource::AddRef()
{
   return InterlockedIncrement(&m_cRef);
}

ULONG ByteSource::Release()
{
   LONG  res = InterlockedDecrement(&m_cRef);
   if (0 == res)
   {
      delete this;
   }
   return res;
}

int ByteSource::GetPinCount()
{
   CAutoLock lock(&mLock);
   return mByteStreams.size();
}

CBasePin* ByteSource::GetPin(int idx)
{
   CAutoLock lock(&mLock);

   if (idx >= 0 && idx < mByteStreams.size())
   {
      vprASSERT(mByteStreams[idx] && "Can't have a null pin.");
      return mByteStreams[idx];
   }
   return NULL;
}

void ByteSource::addPin(ByteStream* pin)
{
   CAutoLock lock(&mLock);
   pin->AddRef();
   mByteStreams.push_back(pin);
}

void ByteSource::removePin(ByteStream* pin)
{
   stream_list_t::iterator found =
      std::find(mByteStreams.begin(), mByteStreams.end(), pin);

   if (mByteStreams.end() != found)
   {
      mByteStreams.erase(found);
      (*found)->Release();
   }
}

// The only allowed way to create Bouncing balls!
ByteStream* ByteStream::create(ByteSource* parent, const vpr::Uint32 width,
                               const vpr::Uint32 height)
{
   return new ByteStream(parent, width, height);
}

ByteStream::ByteStream(ByteSource *parent, const vpr::Uint32 width,
                       const vpr::Uint32 height)
   : CBaseOutputPin(NAME("ByteStreamObj"), parent, parent->lock(), NULL,
                    L"ByteStreamPin")
   , mFilter(parent)
   , mImageWidth(width)
   , mImageHeight(height)
   , mLastTime(0ll)
   , mRepeatTime(20)
{
   CAutoLock cAutoLock(mFilter->lock());
}

ULONG ByteStream::AddRef()
{
   return InterlockedIncrement(&m_cRef);
}

ULONG ByteStream::Release()
{
   LONG  res = InterlockedDecrement(&m_cRef);
   if (0 == res)
   {
      delete this;
   }
   return res;
}

HRESULT ByteStream::CheckMediaType(const CMediaType* mediaType)
{
   CAutoLock lock(mFilter->lock());

   CMediaType mt;
   GetMediaType(0, &mt);

   if (mt == *mediaType)
   {
      return NOERROR;
   }

   return E_FAIL;
}

HRESULT ByteStream::GetMediaType(int idx, CMediaType* mediaType)
{
   CAutoLock lock(mFilter->lock());

   if (idx < 0)
   {
      return E_INVALIDARG;
   }
   if (idx > 0)
   {
      return VFW_S_NO_MORE_ITEMS;
   }

   CheckPointer(mediaType, E_POINTER);
   VIDEOINFO* pvi = (VIDEOINFO*)mediaType->AllocFormatBuffer(sizeof(VIDEOINFO));
   if (NULL == pvi)
   {
      return E_OUTOFMEMORY;
   }

   ZeroMemory(pvi, sizeof(VIDEOINFO));
   pvi->bmiHeader.biCompression = BI_RGB;
   pvi->bmiHeader.biBitCount    = 24;


   // (Adjust the parameters common to all formats...)

   // put the optimal palette in place
   /*
   for(int i = 0; i < iPALETTE_COLORS; i++)
   {
       pvi->TrueColorInfo.bmiColors[i].rgbRed      = m_Palette[i].peRed;
       pvi->TrueColorInfo.bmiColors[i].rgbBlue     = m_Palette[i].peBlue;
       pvi->TrueColorInfo.bmiColors[i].rgbGreen    = m_Palette[i].peGreen;
       pvi->TrueColorInfo.bmiColors[i].rgbReserved = 0;
   }
   */

   pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
   pvi->bmiHeader.biWidth      = mImageWidth;
   pvi->bmiHeader.biHeight     = mImageHeight;
   pvi->bmiHeader.biPlanes     = 1;
   pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
   pvi->bmiHeader.biClrImportant = 0;

   SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
   SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

   mediaType->SetType(&MEDIATYPE_Video);
   mediaType->SetFormatType(&FORMAT_VideoInfo);
   mediaType->SetTemporalCompression(FALSE);

   // Work out the GUID for the subtype from the header info.
   const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
   mediaType->SetSubtype(&SubTypeGUID);
   mediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);

   return NOERROR;
}

HRESULT ByteStream::DecideBufferSize(IMemAllocator* pAlloc,
                                     ALLOCATOR_PROPERTIES* pProperties)
{
   CheckPointer(pAlloc,E_POINTER);
   CheckPointer(pProperties,E_POINTER);

   //CAutoLock cAutoLock(mFilter->lock());
   HRESULT hr = NOERROR;

   VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
   pProperties->cBuffers = 1;
   pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

   vprASSERT(pProperties->cbBuffer);

   // Ask the allocator to reserve us some sample memory, NOTE the function
   // can succeed (that is return NOERROR) but still not have allocated the
   // memory that we requested, so we must check we got whatever we wanted

   ALLOCATOR_PROPERTIES Actual;
   if ( FAILED(hr = pAlloc->SetProperties(pProperties,&Actual)) )
   {
      return hr;
   }

   // Is this allocator unsuitable

   if ( Actual.cbBuffer < pProperties->cbBuffer )
   {
       return E_FAIL;
   }

   // Make sure that we have only 1 buffer (we erase the ball in the
   // old buffer to save having to zero a 200k+ buffer every time
   // we draw a frame)

   vprASSERT(1 == Actual.cBuffers);

   return NOERROR;
}

STDMETHODIMP ByteStream::Notify(IBaseFilter* pSender, Quality q)
{
   // Adjust the repeat rate.
   if ( q.Proportion <= 0 )
   {
      mRepeatTime = 1000;        // We don't go slower than 1 per second
   }
   else
   {
      mRepeatTime = mRepeatTime * 1000 / q.Proportion;
      if ( mRepeatTime > 1000 )
      {
         mRepeatTime = 1000;    // We don't go slower than 1 per second
      }
      else if ( mRepeatTime < 10 )
      {
         mRepeatTime = 10;      // We don't go faster than 100/sec
      }
   }

    // skip forwards
   if ( q.Late > 0 )
   {
      mLastTime += q.Late;
   }

   return NOERROR;
}

ByteStream::~ByteStream()
{
   //CAutoLock cAutoLock(mFilter->lock());
}

void ByteStream::grabFrame(const vpr::Uint32 width, const vpr::Uint32 height,
                           vpr::Uint8* data)
{
   IMediaSample* media_sample;
   CHECK_RESULT(GetDeliveryBuffer(&media_sample, NULL, NULL, 0), 
                "Failed to get a media sample.");

   BYTE* dst_data;
   media_sample->GetPointer(&dst_data);
   long len = media_sample->GetSize();
   ZeroMemory(dst_data, len);
   std::memcpy(dst_data, data, len);

   // The current time is the sample's start
   CRefTime rtStart = mLastTime;
   // Increment to find the finish time
   mLastTime += (LONG)mRepeatTime;
   media_sample->SetTime((REFERENCE_TIME*) &rtStart,
                         (REFERENCE_TIME*) &mLastTime);
   media_sample->SetSyncPoint(TRUE);


   HRESULT hr = Deliver(media_sample);
   media_sample->Release();

   // downstream filter returns S_FALSE if it wants us to
   // stop or an error if it's reporting an error.
   if(hr != S_OK)
   {
      std::ostringstream msg_stream;
      msg_stream << "Failed to deliver media sample (error code " << hr
                 << ")";
      throw BadRecordingData(msg_stream.str(), VRKIT_LOCATION);
   }
   //DeliverEndOfStream();
}

}

}

#endif /* VRKIT_WITH_DIRECT_SHOW */
