// Copyright (C) Infiscape Corporation 2005-2007

#ifdef IOV_WITH_DIRECT_SHOW

#ifndef _INF_DIRECT_SHOW_SOURCE_H_
#define _INF_DIRECT_SHOW_SOURCE_H_

#include <IOV/Config.h>

//#include <streams.h>
//#include <source.h>
//#include <iostream>

//#define DEBUG

#include <atlbase.h>


#include <strsafe.h>
#include <dshow.h>
#include <mtype.h>
#include <wxdebug.h>
#include <reftime.h>
//#pragma comment(lib, "ole32.lib")
#include <atlstr.h>

// CUnknown
#include <combase.h>
// For AutoLock
#include <wxutil.h>
#include <wxlist.h>
#include <amfilter.h>

#include <vpr/vprTypes.h>

#include <vector>


namespace inf
{

template<typename IType>
CComPtr<IType> make(REFCLSID classID, LPUNKNOWN owner,
                    const DWORD classContext, REFIID iid)
{
   CComPtr<IType> obj;
   if ( FAILED(CoCreateInstance(classID, owner, classContext, iid, reinterpret_cast<void**>(&obj))) )
   {
      std::ostringstream msg_stream;
      msg_stream << "Failed to create instance of interface "
                 << typeid(IType).name();
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   return obj;
}

template<typename IType, typename CType>
CComPtr<IType> query(CComPtr<CType> obj, REFIID iid)
{
   CComPtr<IType> inf;
   if ( FAILED(obj->QueryInterface(iid, reinterpret_cast<void**>(&inf))) )
   {
      std::ostringstream msg_stream;
      msg_stream << "Failed to query interface "
                 << typeid(IType).name();
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   return inf;
}

#define createAbstract(type, owner, classContext) \
   (make<I ## type>(CLSID_ ## type, owner, classContext, IID_I ## type))

#define createConcrete(ifType, classType, owner, classContext) \
   (make<ifType>(CLSID_ ## classType, owner, classContext, IID_ ## ifType))

#define query(obj, ifType) \
   (query<ifType>(obj, IID_ ## ifType))

#define CHECK_RESULT(op, msg)                   \
   if ( FAILED(op) )                            \
   {                                            \
      throw inf::Exception(msg, IOV_LOCATION);  \
   }

#define RETURN_FAILED(op)       \
   if (FAILED(hr = op))         \
   { return hr; }

typedef CComPtr<class ByteSource> ByteSourcePtr;
typedef CComPtr<class ByteStream> ByteStreamPtr;

class ByteSource : public CBaseFilter
{
protected:
   ByteSource(LPUNKNOWN owner);
public:
   static ByteSource* create(LPUNKNOWN owner);

   virtual ~ByteSource();

   /** @name IUnknown memory management interface. */
   //@{
   /**
    * Increments reference count.
    */
   STDMETHODIMP_(ULONG) AddRef();

   /**
    * Decrements reference count.
    */
   STDMETHODIMP_(ULONG) Release();
   //@}

   /** @name CBaseFilter Interface */
   //@{
   /**
    * Returns the number of pins for this filter.
    */
   int GetPinCount();

   /**
    * Returns the pin at the given index.
    */
   CBasePin* GetPin(int idx);
   //@}

   /**
    * Returns our critical section lock.
    */
   CCritSec* lock()
   { return &mLock; }

   /** @name Pin management. */
   //@{

   /**
    * Add the given pin to our pins.
    */
   void addPin(ByteStream* pin);

   /**
    * Remove the pin from our pins. If pin is not registered
    * this has no effect.
    */
   void removePin(ByteStream* pin);
   //@}

private:
   typedef std::vector<ByteStream*> stream_list_t;
   stream_list_t        mByteStreams;   /**< List of output pins. */
   CCritSec             mLock;          /**< Critical section lock. */
};

class ByteStream : public CBaseOutputPin
{
protected:
   ByteStream(ByteSource *parent, const vpr::Uint32 width, const vpr::Uint32 height);
public:
   static ByteStream* create(ByteSource* parent, const vpr::Uint32 width,
                           const vpr::Uint32 height);

   virtual ~ByteStream();

   /** @name IUnknown memory management interface. */
   //@{
   /**
    * Increments reference count.
    */
   STDMETHODIMP_(ULONG) AddRef();

   /**
    * Decrements reference count.
    */
   STDMETHODIMP_(ULONG) Release();
   //@}

   /** @name Grabbing interface. */
   //@{
   void grabFrame(const vpr::Uint32 width, const vpr::Uint32 height, vpr::Uint8* data);
   //@}

   /** @name CBaseOutputPin interface. */
   //@{
   // Ask for buffers of the size appropriate to the agreed media type
   HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);
   //@}

   /** @name CBasePin interface. */
   //@{
   virtual HRESULT CheckMediaType(const CMediaType* mediaType);
   virtual HRESULT GetMediaType(int idx, CMediaType* mediaType);
   //@}

   /** @name IQualityControl interface. */
   //@{
   STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);
   //@}

private:
   ByteSource* mFilter; /**< Weak ptr to parent filter. */
   int mImageHeight;    /**< Height of the current image. */
   int mImageWidth;     /**< Width of the current image. */
   CRefTime mLastTime;  /**< Time stamp of last sample. */
   int mRepeatTime;     /**< msec between frames. */
};

}

#endif /*_INF_DIRECT_SHOW_SOURCE_H_*/

#endif /*IOV_WITH_DIRECT_SHOW*/
