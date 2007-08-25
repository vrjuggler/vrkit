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

#ifndef _INF_SIGNAL_DATA_H_
#define _INF_SIGNAL_DATA_H_

#include <IOV/Config.h>

#include <vector>
#include <boost/shared_ptr.hpp>

#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/SignalEmitter.h>


namespace inf
{

/**
 * A signal emitter that exists as IOV scene data.
 *
 * @see inf::SignalEmitter
 *
 * @since 0.23.2
 */
template<typename SlotSignature
       , typename SignalListContainer
       , typename SignalListContainer::signals SignalBegin = SignalListContainer::BEGIN
       , typename SignalListContainer::signals SignalEnd = SignalListContainer::END>
class SignalData
   : public inf::SignalEmitter<SlotSignature
                             , SignalListContainer
                             , SignalBegin
                             , SignalEnd>
   , public inf::SceneData
{
protected:
   SignalData()
       : inf::SignalEmitter<SlotSignature
                          , SignalListContainer
                          , SignalBegin
                          , SignalEnd>()
       , inf::SceneData()
   {
      /* Do nothing. */ ;
   }

public:
   typedef SignalData<SlotSignature
                    , SignalListContainer
                    , SignalBegin
                    , SignalEnd> type;

   typedef boost::shared_ptr<type> ptr_t;
   typedef boost::weak_ptr<type> weak_ptr_t;

   static const vpr::GUID type_guid;

   static ptr_t create()
   {
      return ptr_t(new type());
   }

   ~SignalData()
   {
      /* Do nothing. */ ;
   }
};

}


#endif /* _INF_SIGNAL_DATA_H_ */
