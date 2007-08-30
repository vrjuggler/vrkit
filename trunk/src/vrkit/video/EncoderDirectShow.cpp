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

#include <comdef.h>
#include <algorithm>
#include <sstream>
#include <iterator>

#include <vpr/Util/Assert.h>

#include <vrkit/Exception.h>
#include <vrkit/video/DirectShowSource.h>
#include <vrkit/video/EncoderDirectShow.h>

// Uncomment to display a movie preview window while recording.
//#define PREVIEW


namespace vrkit
{

namespace video
{

CComPtr<IPin> getPin(CComPtr<IBaseFilter> filter, const PIN_DIRECTION dir,
                     int idx)
{
   if ( ! filter )
   {
      return E_POINTER;
   }

   CComPtr<IEnumPins> enum_pins = NULL;
   CComPtr<IPin> pin = NULL;

   HRESULT hr;
   if ( FAILED(hr = filter->EnumPins(&enum_pins)) )
   {
      return pin;
   }

   ULONG num_found;
   while ( S_OK == enum_pins->Next(1, &pin, &num_found) )
   {
      PIN_DIRECTION pindir = static_cast<PIN_DIRECTION>(3);
      pin->QueryDirection(&pindir);
      if(pindir == dir)
      {
         if(idx == 0)
         {
            // Return the pin's interface
            return pin;
         }
         --idx;
      } 
      pin = NULL;
   }
   return pin;
}

CComPtr<IPin> getInPin(CComPtr<IBaseFilter> filter, const int idx)
{
   return getPin(filter, PINDIR_INPUT, idx);
}

CComPtr<IPin> getOutPin(CComPtr<IBaseFilter> filter, const int idx)
{
   return getPin(filter, PINDIR_OUTPUT, idx);
}

// Adds a DirectShow filter graph to the Running Object Table,
// allowing GraphEdit to "spy" on a remote filter graph.
DWORD addGraphToRunningObjTable(IUnknown* graph) 
{
   if ( ! graph )
   {
      return E_POINTER;
   }

   DWORD regId;
   CComPtr<IMoniker> moniker;
   CComPtr<IRunningObjectTable> rot;

   CHECK_RESULT(GetRunningObjectTable(0, &rot),
                "Failed to get running object table.");

   WCHAR graph_reg_str[128];
   CHECK_RESULT(StringCchPrintfW(graph_reg_str, NUMELMS(graph_reg_str),
                                 L"FilterGraph %08x pid %08x\0",
                                 (DWORD_PTR)graph, GetCurrentProcessId()),
                "Failed to create graph registration str.")

   CHECK_RESULT(CreateItemMoniker(L"!", graph_reg_str, &moniker),
                "Failed to create item moniker");

   // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
   // to the object.  Using this flag will cause the object to remain
   // registered until it is explicitly revoked with the Revoke() method.
   //
   // Not using this flag means that if GraphEdit remotely connects
   // to this graph and then GraphEdit exits, this object registration 
   // will be deleted, causing future attempts by GraphEdit to fail until
   // this application is restarted or until the graph is registered again.
   CHECK_RESULT(
      rot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, graph, moniker, &regId),
      "Failed to register graph with running object table."
   );

   return regId;
}

// Removes a filter graph from the Running Object Table
void removeGraphFromRunningObjTable(const DWORD regId)
{
   CComPtr<IRunningObjectTable> rot;

   CHECK_RESULT(GetRunningObjectTable(0, &rot),
                "Failed to get running object table.");

   rot->Revoke(regId);
}

EncoderDirectShow::EncoderDirectShow()
   : mGraphBuilder(NULL)
   , mMediaController(NULL)
   , mByteSource(NULL)
   , mByteStream(NULL)
   , mRunning(false)
#ifdef REGISTER_GRAPH
   , mGraphRegister(0)
#endif
{
   // Initialize COM
   if ( FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) )
   {
      throw Exception("Could not initialize COM", VRKIT_LOCATION);
   }
}

EncoderPtr EncoderDirectShow::create()
{
   return EncoderPtr(new EncoderDirectShow);
}

EncoderDirectShow::~EncoderDirectShow()
{
   // Be sure that we are no longer encoding before calling CoUninitialize().
   // The value of mRunning helps ensure that this call will not cause bad
   // things to happen.
   stopEncoding();

   // We are finished with COM.
   CoUninitialize();
}

EncoderPtr EncoderDirectShow::init()
{
   // TODO: Support ASF (MEDIASUBTYPE_Asf) and other container formats that
   // can be handled by DirectShow. Figuring out what those other formats are
   // seems to be the hardest part.
   container_format_info_t format_info;
   format_info.mFormatName = "AVI";
   format_info.mFormatLongName = "Audio Video Interleave";
   format_info.mFileExtensions.push_back("avi");
   format_info.mCodecList = EncoderDirectShow::getCodecs();
   format_info.mEncoderName = EncoderDirectShow::getName();

   mContainerFormatInfoList.push_back(format_info);

   return shared_from_this();
}

void EncoderDirectShow::startEncoding()
{
   HRESULT hr;

   // Get the interface for DirectShow's GraphBuilder
   mGraphBuilder = createConcrete(IGraphBuilder, FilterGraph, NULL,
                                  CLSCTX_INPROC_SERVER);

   // Get the media controller interface from graph builder.
   mMediaController = query(mGraphBuilder, IMediaControl);

   // Create ByteSource filter to capture OpenGL.
   mByteSource = ByteSource::create(NULL);
   mByteSource->AddRef();

   // Create ByteStream to capture OpenGL.
   mByteStream = ByteStream::create(mByteSource, getWidth(), getHeight());
   mByteStream->AddRef();
   mByteSource->addPin(mByteStream);

   // Add the source to the graph.
   CHECK_RESULT(mGraphBuilder->AddFilter(mByteSource, L"ByteSource"),
                "Can't add ByteSource filter");

   // Create a capture graph builder to help.
   CComPtr<ICaptureGraphBuilder2> capture_graph =
      createConcrete(ICaptureGraphBuilder2, CaptureGraphBuilder2, NULL,
                     CLSCTX_INPROC_SERVER);

   // Tell the capture graph about the graph builder that we are using.
   capture_graph->SetFiltergraph(mGraphBuilder);

   CComPtr<IBaseFilter> avi_mux = NULL;
   // Use the help of CaptureGraph to build everything we need to write to the
   // given file.
   hr = capture_graph->SetOutputFileName(&MEDIASUBTYPE_Avi,
                                         _bstr_t(getFilename().c_str()),
                                         &avi_mux, NULL);
   setAviOptions(avi_mux, INTERLEAVE_NONE);

#ifdef PREVIEW
   CComPtr<IBaseFilter> smart_tee =
      createConcrete(IBaseFilter, SmartTee, NULL, CLSCTX_INPROC_SERVER);
   CHECK_RESULT(mGraphBuilder->AddFilter(smart_tee, L"SmartTee"),
                "Failed to add smart tee!");

   CComPtr<IBaseFilter> video_render =
      createConcrete(IBaseFilter, VideoRenderer, NULL, CLSCTX_INPROC_SERVER);
   CHECK_RESULT(mGraphBuilder->AddFilter(video_render, L"Preview Window"),
                "Failed to add renderer!");
#endif

   CComPtr<IBaseFilter> encoder = getEncoder(getCodec());

   if ( ! encoder )
   {
      std::ostringstream msg_stream;
      msg_stream << "Failed to get COM encoder object for codec '"
                 << getCodec() << "'";
      throw Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   CHECK_RESULT(mGraphBuilder->AddFilter(encoder, L"Encoder"),
                "Failed to add encoder");

   CComPtr<IPin> encoder_in = getInPin(encoder,0);
   CComPtr<IPin> encoder_out = getOutPin(encoder,0);
   CComPtr<IPin> avimux_in = getInPin(avi_mux,0);
   CComPtr<IPin> byte_source_out = mByteSource->GetPin(0);

#ifdef PREVIEW
   CComPtr<IPin> tee_in = getInPin(smart_tee, 0);
   CComPtr<IPin> tee_o1 = NULL;
   CComPtr<IPin> tee_o2 = NULL;
   smart_tee->FindPin(L"Preview", &tee_o1);
   smart_tee->FindPin(L"Capture", &tee_o2);

   CHECK_RESULT(mGraphBuilder->Connect(byte_source_out, tee_in),
                "Failed to connect input to smart tee.");
   CHECK_RESULT(mGraphBuilder->Connect(tee_o2, encoder_in),
                "Failed to connect desktop to AviMux!");
#else
   CHECK_RESULT(
      mGraphBuilder->Connect(byte_source_out, encoder_in),
      "Failed to connect byte source output pin to encoder input pin!"
   );
#endif
   CHECK_RESULT(mGraphBuilder->Connect(encoder_out, avimux_in),
                "Failed to connect encoder output pin to AviMux!");

#ifdef PREVIEW
   mGraphBuilder->Render(tee_o1);
#endif

   CHECK_RESULT(mMediaController->Run(), "Failed to run DirectShow Error!");
   mRunning = true;

#ifdef REGISTER_GRAPH
   mGraphRegister = addGraphToRunningObjTable(mGraphBuilder);
#endif
}

void EncoderDirectShow::stopEncoding()
{
   if ( mRunning )
   {
      CHECK_RESULT(mMediaController->StopWhenReady(),
                   "Failed to stop DirectShow.");

#ifdef REGISTER_GRAPH
      if ( mGraphRegister )
      {
         removeGraphFromRunningObjTable(mGraphRegister);
         mGraphRegister = 0;
      }
#endif
   }

   // Clean up member data we don't need anymore.
   mGraphBuilder = NULL;
   mMediaController = NULL;

   // Clean up our filter and source pin.
   if ( NULL != mByteSource )
   {
      mByteSource->Release();
      mByteSource = NULL;
   }

   if ( NULL != mByteStream )
   {
      mByteStream->Release();
      mByteStream = NULL;
   }
}

void EncoderDirectShow::writeFrame(vpr::Uint8* data)
{
   mByteStream->grabFrame(getWidth(), getHeight(), data);
}

void EncoderDirectShow::setAviOptions(CComPtr<IBaseFilter> filter,
                                      const InterleavingMode mode)
{
   vprASSERT(filter);
   if ( ! filter )
   {
      throw Exception("Can't have NULL AVI Mux.", VRKIT_LOCATION);
   }

   // QI for interface AVI Muxer
   CComPtr<IConfigAviMux> avi_mux = query(filter, IConfigAviMux);
   CHECK_RESULT(avi_mux->SetOutputCompatibilityIndex(TRUE),
                "avi_mux->SetOutputCompatibilityIndex failed.");

   // QI for interface Interleaving
   CComPtr<IConfigInterleaving> interleaving = query(filter,
                                                     IConfigInterleaving);
   // put the interleaving mode (full, none, half)
   CHECK_RESULT(interleaving->put_Mode(mode),
                "interleaving->put_Mode failed.");
} 

EncoderDirectShow::moniker_list_t EncoderDirectShow::getVideoMonikers()
{
   moniker_list_t monikers;

   CComPtr<IEnumMoniker> enum_moniker = NULL;
   CComPtr<IMoniker> moniker = NULL;

   CComPtr<ICreateDevEnum> sys_dev_enum =
      createConcrete(ICreateDevEnum, SystemDeviceEnum, NULL,
                     CLSCTX_INPROC_SERVER);

   // S_FALSE means nothing in this category.
   if ( S_OK == sys_dev_enum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &enum_moniker, 0) )
   {
      while ( S_OK == enum_moniker->Next(1, &moniker, NULL) )
      {
         monikers.push_back(moniker);
         moniker = NULL;
      }
   }

   return monikers;
}

EncoderDirectShow::codec_list_t EncoderDirectShow::getCodecs()
{
   // Get a list of all video monikers so that we can grab the codec names.
   moniker_list_t monikers = getVideoMonikers();
   codec_list_t encoder_names;

   // Iterate over monikers trying to grab the friendly name.
   typedef moniker_list_t::iterator iter_type;
   for ( iter_type m = monikers.begin(); m != monikers.end(); ++m )
   {
      CComPtr<IPropertyBag> prop_bag = NULL;
      (*m)->BindToStorage(0, 0, IID_IPropertyBag,
                          reinterpret_cast<void**>(&prop_bag));
      VARIANT var;
      VariantInit(&var);
      if ( SUCCEEDED(prop_bag->Read(L"FriendlyName", &var, 0)) )
      {
         std::string name = _bstr_t(var.bstrVal);
         encoder_names.push_back(name);
      }
      VariantClear(&var); 
   }
   return encoder_names;
}

void EncoderDirectShow::printVideoEncoders()
{
   const std::vector<std::string> encoder_names = getCodecs();
   std::copy(encoder_names.begin(), encoder_names.end(),
             std::ostream_iterator<std::string>(std::cout, "\n"));
   std::cout << std::flush;
}

void EncoderDirectShow::printPins(CComPtr<IBaseFilter> filter)
{
   CComPtr<IEnumPins> pins;
   ULONG n;

   CHECK_RESULT(filter->EnumPins(&pins), "Failed to enum pins.");

   CComPtr<IPin> pin;
   while (pins->Next(1, &pin, &n) == S_OK)
   {
      PIN_INFO pin_info;
      pin->QueryPinInfo(&pin_info);
      std::cout << "pin: " << _bstr_t(pin_info.achName) << std::endl;
      QueryPinInfoReleaseFilter(pin_info);
      pin = NULL;
   }
}

CComPtr<IBaseFilter> EncoderDirectShow::getEncoder(const std::string& name)
{
   HRESULT hr;
   moniker_list_t monikers = getVideoMonikers();

   typedef moniker_list_t::iterator iter_type;
   for ( iter_type m = monikers.begin(); m != monikers.end(); ++m )
   {
      CComPtr<IPropertyBag> prop_bag = NULL;
      (*m)->BindToStorage(0, 0, IID_IPropertyBag,
                          reinterpret_cast<void**>(&prop_bag));
      VARIANT var;
      VariantInit(&var);
      hr = prop_bag->Read(L"FriendlyName", &var, 0);
      if (SUCCEEDED(hr))
      {
         std::string enc_name = _bstr_t(var.bstrVal);
         if (enc_name == name)
         {
            CComPtr<IBaseFilter> filter = NULL;
            hr = (*m)->BindToObject(NULL, NULL, IID_IBaseFilter,
                                    reinterpret_cast<void**>(&filter));

            if (SUCCEEDED(hr))
            {
               return filter;
            }
         }
      }
      VariantClear(&var); 
   }
   CComPtr<IBaseFilter> filter = NULL;
   return filter;
}

}

}

#endif /* VRKIT_WITH_DIRECT_SHOW */
