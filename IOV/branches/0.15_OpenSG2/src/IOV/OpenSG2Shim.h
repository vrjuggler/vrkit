// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_OPENSG2_SHIM_H_
#define _INF_OPENSG2_SHIM_H_

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGConfigured.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGNodeCore.h>
#include <OpenSG/OSGTextureObjChunk.h>
#include <boost/bind.hpp>

#if OSG_MAJOR_VERSION >= 2

#define OPENSG2SHIM 1
namespace OSG {

typedef TextureObjChunkPtr TextureChunkPtr;

#if 1
template< class Core > inline
void beginEditCP(const CoredNodePtr<Core> &objectP, 
                       BitVector         whichField = FieldBits    ::AllFields,
                       UInt32            origin     = ChangedOrigin::External)
                       {}

template< class Core > inline
void endEditCP  (const CoredNodePtr<Core> &objectP, 
                       BitVector         whichField = FieldBits    ::AllFields,
                       UInt32            origin     = ChangedOrigin::External)
                       {commitChanges();}
#endif

#if 0
void beginEdit        (BitVector whichField = FieldBits::AllFields,
			UInt32 origin = ChangedOrigin::External)
    {}

void endEdit          (BitVector whichField = FieldBits::AllFields,
			UInt32 origin = ChangedOrigin::External)
    {}
#endif

#if 0
typedef void CPEditor (const FieldContainerPtr &objectP, 
                               BitVector whichField = FieldBits::AllFields,
                               UInt32    origin     = ChangedOrigin::External);
#endif

#if 1
inline void beginEditCP        (const FieldContainerPtr &objectP, 
                               BitVector whichField = FieldBits::AllFields,
                               UInt32    origin     = ChangedOrigin::External)
                               {}

inline void endEditCP          (const FieldContainerPtr &objectP, 
                               BitVector whichField = FieldBits::AllFields,
                               UInt32    origin     = ChangedOrigin::External)
                                   {commitChanges();}
#endif
/*
struct CorePtr;
struct CorePtr::StoredObjectType;

static const OSG::BitVector CorePtr::StoredObjectType::MaterialFieldMask =
	    (TypeTraits<BitVector>::One);
*/

class CPEditor
{
    /*==========================  PUBLIC  =================================*/

  public:

    /*---------------------------------------------------------------------*/
    /*! \name                   Constructors                               */
    /*! \{                                                                 */
  
    inline CPEditor(void) {}
    
    inline CPEditor(const FieldContainerPtr fc, 
                    const BitVector mask = FieldBits::AllFields) {}

    /*! \}                                                                 */
    /*---------------------------------------------------------------------*/
    /*! \name                   Destructor                                 */
    /*! \{                                                                 */
    
    inline ~CPEditor() {}

    /*! \}                                                                 */
    /*==========================  PRIVATE  ================================*/

  private:
  
    FieldContainerPtr   _fc;
    BitVector           _mask;
};

}
#else
namespace OSG {
//Define NodePtrConstArg to NodePtr&
typedef NodePtr &NodePtrConstArg;
typedef FieldContainerPtr &FieldContainerPtrConstArg;
}
#endif

#endif
