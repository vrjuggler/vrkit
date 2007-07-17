#ifndef _INF_SIGNAL_DATA_H_
#define _INF_SIGNAL_DATA_H_

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
class IOV_CLASS_API SignalData
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
