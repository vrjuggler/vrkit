#ifdef IOV_WITH_DIRECT_SHOW

#include <sstream>
#include <IOV/Util/Exceptions.h>
#include <IOV/Video/DirectShowEncoder.h>

#include <comdef.h>

#define PREVIEW

namespace inf
{

//void f()
//{
//   createAbstract(ClassGraphBuilder2, NULL, 1);
//   createConcrete(GraphBuilder, FilterGraph, NULL, 1);
//}

DirectShowEncoder::DirectShowEncoder()
   : mWidth(512)
   , mHeight(512)
   , mGraphBuilder(NULL)
   , mMediaController(NULL)
   , mByteSource(NULL)
   , mByteStream(NULL)
#ifdef REGISTER_GRAPH
   , mGraphRegister(0)
#endif
{
}

EncoderPtr DirectShowEncoder::create()
{
   return EncoderPtr(new DirectShowEncoder);
}

DirectShowEncoder::~DirectShowEncoder()
{
}

EncoderPtr DirectShowEncoder::init(const std::string& filename, const std::string& codec,
                                   const vpr::Uint32 width, const vpr::Uint32 height,
                                   const vpr::Uint32 framesPerSecond)
{
   HRESULT hr;

   // Get the interface for DirectShow's GraphBuilder
   mGraphBuilder = createConcrete(IGraphBuilder, FilterGraph, NULL, CLSCTX_INPROC_SERVER);

   // Get the media controller interface from graph builder.
   mMediaController = query(mGraphBuilder, IMediaControl);

   // Create ByteSource filter to capture OpenGL.
   mByteSource = ByteSource::create(NULL);
   mByteSource->AddRef();

   // Create ByteStream to capture OpenGL.
   mByteStream = ByteStream::create(mByteSource, width, height);
   mByteStream->AddRef();
   mByteSource->addPin(mByteStream);

   // Add the source to the graph.
   CHECK_RESULT(mGraphBuilder->AddFilter(mByteSource, L"ByteSource"), "Can't add ByteSource filter");

   // Create a capture graph builder to help.
   CComPtr<ICaptureGraphBuilder2> capture_graph =
      createConcrete(ICaptureGraphBuilder2, CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER);

   // Tell the capture graph about the graph builder that we are using.
   capture_graph->SetFiltergraph(mGraphBuilder);

   CComPtr<IBaseFilter> avi_mux = NULL;
   // Use the help of CaptureGraph to build everything we need to write to the given file.
   hr = capture_graph->SetOutputFileName(&MEDIASUBTYPE_Avi, _bstr_t(filename.c_str()), &avi_mux, NULL);
   setAviOptions(avi_mux, INTERLEAVE_NONE);

#ifdef PREVIEW
   CComPtr<IBaseFilter> smart_tee =
      createConcrete(IBaseFilter, SmartTee, NULL, CLSCTX_INPROC_SERVER);
   CHECK_RESULT(mGraphBuilder->AddFilter(smart_tee, L"SmartTee"), "Failed to add smart tee!");

   CComPtr<IBaseFilter> video_render =
      createConcrete(IBaseFilter, VideoRenderer, NULL, CLSCTX_INPROC_SERVER);
   CHECK_RESULT(mGraphBuilder->AddFilter(video_render, L"Preview Window"), "Failed to add renderer!");
#endif

   CComPtr<IBaseFilter> encoder = getEncoder("Microsoft MPEG-4  VKI  Codec V3");

   CHECK_RESULT(mGraphBuilder->AddFilter(encoder, L"Encoder"), "Failed to add encoder");

   CComPtr<IPin> encoder_in = GetInPin(encoder,0);
   CComPtr<IPin> encoder_out = GetOutPin(encoder,0);
   CComPtr<IPin> avimux_in = GetInPin(avi_mux,0);
   CComPtr<IPin> byte_source_out = mByteSource->GetPin(0);

#ifdef PREVIEW
   CComPtr<IPin> tee_in = GetInPin(smart_tee, 0);
   CComPtr<IPin> tee_o1 = NULL;
   CComPtr<IPin> tee_o2 = NULL;
   smart_tee->FindPin(L"Preview", &tee_o1);
   smart_tee->FindPin(L"Capture", &tee_o2);

   CHECK_RESULT(mGraphBuilder->Connect(byte_source_out, tee_in), "Failed to connect input to smart tee.");
   CHECK_RESULT(mGraphBuilder->Connect(tee_o2, encoder_in), "Failed to connect desktop to AviMux!");
#else
   CHECK_RESULT(mGraphBuilder->Connect(byte_source_out, encoder_in), "Failed to connect desktop to AviMux!");
#endif
   CHECK_RESULT(mGraphBuilder->Connect(encoder_out, avimux_in), "Failed to connect desktop to AviMux!");

#ifdef PREVIEW
   mGraphBuilder->Render(tee_o1);
#endif

   CHECK_RESULT(hr = mMediaController->Run(), "Failed to run DirectShow Error: ");

#ifdef REGISTER_GRAPH
   if (FAILED(AddGraphToRot(mGraphBuilder, &mGraphRegister)))
   {
      throw inf::Exception("Failed to register filter graph with ROT!", IOV_LOCATION);
      mGraphRegister = 0;
   }
#endif

   return shared_from_this();
}

void DirectShowEncoder::writeFrame(int width, int height, vpr::Uint8* data)
{
   mByteStream->grabFrame(width, height, data);
}

#define PREVIEW

void DirectShowEncoder::setAviOptions(IBaseFilter *ppf, InterleavingMode INTERLEAVE_MODE)
{
   CComPtr<IConfigAviMux>        avi_mux           = NULL;
   CComPtr<IConfigInterleaving>  interleaving  = NULL;

   ASSERT(ppf);
   if (!ppf)
   {
      throw inf::Exception("Can't have NULL AVI Mux.", IOV_LOCATION);
   }
   // QI for interface AVI Muxer
   if (FAILED(ppf->QueryInterface(IID_IConfigAviMux, reinterpret_cast<PVOID *>(&avi_mux))))
   {
      throw inf::Exception("CDVGraph::SetAviOptions::QI IConfigAviMux failed.", IOV_LOCATION);
   }

   if (FAILED(avi_mux->SetOutputCompatibilityIndex(TRUE)))
   {
      throw inf::Exception("CDVGraph::SetAviOptions::avi_mux->SetOutputCompatibilityIndex failed.", IOV_LOCATION);
   }

   // QI for interface Interleaving
   if (FAILED(ppf->QueryInterface(IID_IConfigInterleaving, reinterpret_cast<PVOID *>(&interleaving))))
   {
      throw inf::Exception("CDVGraph::SetAviOptions::QI IConfigInterleaving failed.", IOV_LOCATION);
   }

   // put the interleaving mode (full, none, half)
   if (FAILED(interleaving->put_Mode(INTERLEAVE_MODE)))
   {
      throw inf::Exception("CDVGraph::SetAviOptions::interleaving->put_Mode failed.", IOV_LOCATION);
   }
} 

DirectShowEncoder::moniker_list_t DirectShowEncoder::getVideoMonikers()
{
   moniker_list_t monikers;

   CComPtr<IEnumMoniker> enum_moniker = NULL;
   CComPtr<IMoniker> pMoniker = NULL;

   CComPtr<ICreateDevEnum> sys_dev_enum =
      createConcrete(ICreateDevEnum, SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER);

   // S_FALSE means nothing in this category.
   if (S_OK == sys_dev_enum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &enum_moniker, 0))
   {
      while (S_OK == enum_moniker->Next(1, &pMoniker, NULL))
      {
         monikers.push_back(pMoniker);
         pMoniker = NULL;
      }
   }

   return monikers;
}

struct ComInit
{
   ComInit()
   {
      // Initialize COM
      if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
      {
         throw inf::Exception("Could not initialize COM", IOV_LOCATION);
      }
   }
   ~ComInit()
   {
      // XXX: Finished with COM
      CoUninitialize();
   }
};

DirectShowEncoder::codec_list_t DirectShowEncoder::getCodecs()
{
   static ComInit com_init_hack;

   // Get a list of all video monikers so that we can grab the codec names.
   moniker_list_t monikers = getVideoMonikers();
   codec_list_t encoder_names;

   // Iterate over monikers trying to grab the friendly name.
   for (moniker_list_t::iterator m = monikers.begin(); m != monikers.end(); ++m)
   {
      CComPtr<IPropertyBag> pPropBag = NULL;
      (*m)->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
      VARIANT var;
      VariantInit(&var);
      if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &var, 0)))
      {
         std::string name = _bstr_t(var.bstrVal);
         encoder_names.push_back(name);
      }
      VariantClear(&var); 
   }
   return encoder_names;
}

void DirectShowEncoder::printVideoEncoders()
{
   std::vector<std::string> encoder_names;
   encoder_names = getCodecs();
   for (std::vector<std::string>::iterator itr = encoder_names.begin();
        itr != encoder_names.end(); ++itr)
   {
      std::cout << *itr << std::endl;
   }
}

void DirectShowEncoder::printPins(CComPtr<IBaseFilter> filter)
{
   CComPtr<IEnumPins> pins;
   ULONG n;

   CHECK_RESULT(filter->EnumPins(&pins), "Failed to enum pins.");

   CComPtr<IPin> pin;
   while (pins->Next(1, &pin, &n) == S_OK)
   {
      PIN_INFO pi;
      pin->QueryPinInfo(&pi);
      std::cout << "pin: " << _bstr_t(pi.achName) << std::endl;
      QueryPinInfoReleaseFilter(pi);
      pin = NULL;
   }
}

CComPtr<IBaseFilter> DirectShowEncoder::getEncoder(const std::string name)
{
   HRESULT hr;
   moniker_list_t monikers = getVideoMonikers();

   for (moniker_list_t::iterator m = monikers.begin(); m != monikers.end(); ++m)
   {
      CComPtr<IPropertyBag> pPropBag = NULL;
      (*m)->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
      VARIANT var;
      VariantInit(&var);
      hr = pPropBag->Read(L"FriendlyName", &var, 0);
      if (SUCCEEDED(hr))
      {
         std::string enc_name = _bstr_t(var.bstrVal);
         if (enc_name == name)
         {
            CComPtr<IBaseFilter> filter = NULL;
            hr = (*m)->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&filter);
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

void DirectShowEncoder::close()
{
   CHECK_RESULT(mMediaController->StopWhenReady(), "Failed to stop direct show.");

#ifdef REGISTER_GRAPH
   if (mGraphRegister)
   {
      RemoveGraphFromRot(mGraphRegister);
      mGraphRegister = 0;
   }
#endif

   // Clean up our filter and source pin.
   mByteSource->Release();
   mByteSource = NULL;
   mByteStream->Release();
   mByteStream = NULL;
}

}

#endif /*IOV_WITH_DIRECT_SHOW*/
