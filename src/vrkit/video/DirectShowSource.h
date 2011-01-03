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

#ifdef VRKIT_WITH_DIRECT_SHOW

#ifndef _VRKIT_VIDEO_DIRECT_SHOW_SOURCE_H_
#define _VRKIT_VIDEO_DIRECT_SHOW_SOURCE_H_

#include <vrkit/Config.h>

#include <atlbase.h>
#include <strmif.h>     /* Must be included before combase.h */
#include <streams.h>    /* Ditto */
// CUnknown
#include <combase.h>    /* Must be included before amfilter.h */
#include <amfilter.h>   /* For DirectShow Base Classes types */

#include <sstream>
#include <vector>
#include <typeinfo>

#include <vpr/vprTypes.h>
#include <vpr/vprParam.h>
#include <vpr/Util/Error.h>

#include <vrkit/Exception.h>


namespace vrkit
{

namespace video
{

/**
 * Creates an instance of a COM interface type, provided as the template
 * parameter.
 *
 * @param classID      The class identifier associated with the information
 *                     that will be used to create the object.
 * @param owner        The owner of the object to be created if it is to be
 *                     created as part of an aggregate. Pass NULL to indicate
 *                     that the object is not part of an aggregate.
 * @param classContext The context in which the code creating the instance
 *                     will run. Values come from the CLSCTX enumeration.
 * @param iid          The interface identifier for the object to be created.
 *
 * @reutrn A newly created instance of \p IType is returned to the caller.
 *
 * @throw vrkit::Exception Thrown if an instance of the identified interface
 *                         cannot be constructed.
 *
 * @note This was moved into the vrkit::video namespace in version 0.47.
 *
 * @since 0.44
 */
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
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   return obj;
}

/**
 * Queries the interface of the given object to provide an alternate means for
 * communicating with it. The returned object implements the interface being
 * queried.
 *
 * @param obj The object whose interfaces will be queried.
 * @param iid The identifer of the interface being requested from \p obj.
 *
 * @return A COM pointer implementing \p IType is returned to the caller.
 *
 * @throw vrkit::Exception Thrown if the desired interface is not supported by
 *                         \p obj.
 *
 * @note This was moved into the vrkit::video namespace in version 0.47.
 *
 * @since 0.44
 */
template<typename IType, typename CType>
CComPtr<IType> query(CComPtr<CType> obj, REFIID iid)
{
   CComPtr<IType> inf;
   if ( FAILED(obj->QueryInterface(iid, reinterpret_cast<void**>(&inf))) )
   {
      std::ostringstream msg_stream;
      msg_stream << "Failed to query interface " << typeid(IType).name();
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   return inf;
}

/**
 * @name Helper Macros
 *
 * These macros provide simpler access to vrkit::make<T>(), vrkit::query<T>(),
 * and common operations performed in Windows programming.
 *
 * @since 0.44
 */
//@{
/**
 * Creates an instance of the given abstract type. The abstract type is passed
 * in as the first parameter to the macro. This type has corresponding class
 * and interface identifiers. For example, consider the interface
 * \c ISomeInterface with class ID \c CLSID_SomeInterface and interface ID
 * \c IID_ISomeInterface. This macro is then used as follows:
 *
 * \code
 * CComPtr<ISomeInterface> obj = createAbstract(SomeInterface, NULL,
 *                                              CLSCTX_INPROC_SERVER);
 * \endcode
 *
 * @see vrkit::make<T>()
 */
#define createAbstract(type, owner, classContext) \
   (make<I ## type>(CLSID_ ## type, owner, classContext, IID_I ## type))

/**
 * Creates an instance of a concrete implementation of an interface. The base
 * interface type is passed in as the first parameter to the macro, and the
 * concrete implementation type is passed in as the second. The interface type
 * has an interface identifer, and the concrete type has a class identifier.
 * For example, consider the interface \c ISomeInterface implemented by a
 * class \c CMyClass. The interface has the ID \c IID_ISomeInterface, and the
 * implementation has the class ID \c CLSID_CMyClass. This macro is then used
 * as follows:
 *
 * \code
 * CComPtr<ISomeInterface> obj = createConcrete(SomeInterface, CMyClass, NULL,
 *                                              CLSCTX_INPROC_SERVER);
 * \endcode
 *
 * @see vrkit::make<T>()
 */
#define createConcrete(ifType, classType, owner, classContext) \
   (make<ifType>(CLSID_ ## classType, owner, classContext, IID_ ## ifType))

/**
 * Wraps a call to vrkit::query<T>() by filling in the template parameter and
 * the interface identifier for the abstract interface. The abstract type is
 * passed in as the first parameter to the macro. For example, consider the
 * interface \c ISomeInterface with class ID interface ID
 * \c IID_ISomeInterface. This macro is then used as follows:
 *
 * \code
 * CComPtr<ISomeInterface> obj2 = query(obj1, SomeInterface);
 * \endcode
 *
 * @see vrkit::query<T>()
 */
#define query(obj, ifType) \
   (query<ifType>(obj, IID_ ## ifType))

#if __VPR_version >= 1001006
/**
 * Translates the use of FAILED() into a vrkit::Exception.
 *
 * @note vpr::Error::getCurrentErrorMsg() was added in VPR 1.1.6.
 */
#define CHECK_RESULT(op, msg)                                           \
   if ( FAILED(op) )                                                    \
   {                                                                    \
      std::ostringstream msg_stream;                                    \
      msg_stream << msg;                                                \
      const std::string err_msg = vpr::Error::getCurrentErrorMsg();     \
      if ( ! err_msg.empty() )                                          \
      {                                                                 \
         msg_stream << std::endl << err_msg;                            \
      }                                                                 \
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);         \
   }
#else
/**
 * Translates the use of FAILED() into a vrkit::Exception.
 */
#define CHECK_RESULT(op, msg)                                           \
   if ( FAILED(op) )                                                    \
   {                                                                    \
      throw vrkit::Exception(msg, VRKIT_LOCATION);                      \
   }
#endif

#define RETURN_FAILED(op)       \
   if ( FAILED(hr = op) )       \
   {                            \
      return hr;                \
   }
//@}

typedef CComPtr<class ByteSource> ByteSourcePtr;
typedef CComPtr<class ByteStream> ByteStreamPtr;

/** \class ByteSource DirectShowSource.h vrkit/video/DirectShowSource.h
 *
 * Provides a filter accepting an array of bytes as input to a DirectShow
 * filter graph.
 *
 * @note This was moved into the vrkit::video namespace in version 0.47.
 *
 * @since 0.44
 */
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
   {
      return &mLock;
   }

   /** @name Pin Management */
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

/** \class ByteStream DirectShowSource.h vrkit/video/DirectShowSource.h
 *
 * Provides an output pin for vrkit::ByteSource. This is where the work of
 * translating the array of bytes (typically from an OpenGL frame buffer) is
 * done.
 *
 * @note This was moved into the vrkit::video namespace in version 0.47.
 *
 * @since 0.44
 */
class ByteStream : public CBaseOutputPin
{
protected:
   ByteStream(ByteSource *parent, const vpr::Uint32 width,
              const vpr::Uint32 height);

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
   void grabFrame(const vpr::Uint32 width, const vpr::Uint32 height,
                  vpr::Uint8* data);
   //@}

   /** @name CBaseOutputPin interface. */
   //@{
   // Ask for buffers of the size appropriate to the agreed media type
   HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc,
                            ALLOCATOR_PROPERTIES* pProperties);
   //@}

   /** @name CBasePin interface. */
   //@{
   virtual HRESULT CheckMediaType(const CMediaType* mediaType);
   virtual HRESULT GetMediaType(int idx, CMediaType* mediaType);
   //@}

   /** @name IQualityControl interface. */
   //@{
   STDMETHODIMP Notify(IBaseFilter* pSender, Quality q);
   //@}

private:
   ByteSource* mFilter; /**< Weak ptr to parent filter. */
   int mImageHeight;    /**< Height of the current image. */
   int mImageWidth;     /**< Width of the current image. */
   CRefTime mLastTime;  /**< Time stamp of last sample. */
   int mRepeatTime;     /**< msec between frames. */
};

}

}


#endif /* _VRKIT_VIDEO_DIRECT_SHOW_SOURCE_H_ */

#endif /* VRKIT_WITH_DIRECT_SHOW */
