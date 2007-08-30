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
