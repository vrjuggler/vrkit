// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRAB_DATA_H_
#define _INF_GRAB_DATA_H_

#include <IOV/Config.h>

#include <vector>
#include <boost/signal.hpp>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/GrabDataPtr.h>
#include <IOV/SceneObjectPtr.h>


namespace inf
{

/**
 * Centralized storage for all the scene objects that are currently
 * eligible for grabbing.
 */
class IOV_CLASS_API GrabData : public inf::SceneData
{
public:
   typedef boost::signal<void (inf::SceneObjectPtr)> signal_t;

   typedef std::vector<SceneObjectPtr> object_list_t;

   /**
    * The unique type identifier for scene data used by plug-ins that manage
    * grabbing of objects in the scene.
    */
   static const vpr::GUID type_guid;

   static GrabDataPtr create()
   {
      return GrabDataPtr(new GrabData);
   }

   virtual ~GrabData();

   /**
    * Connects the given slot to the signal emitted when a grabbable object
    * is added. The connection created between the signal and the slot is
    * returned to the caller. It is up to the caller to manage the connection.
    *
    * @post \p slot is connected to the object addition signal.
    *
    * @param slot The slot to be connected to the object addition signal.
    *
    * @return The object holding the connection between the object addition
    *         signal and \p slot.
    *
    * @see addObject()
    *
    * @since 0.23.1
    */
   boost::signals::connection connectToAdd(signal_t::slot_type slot);

   /**
    * Connects the given slot to the signal emitted when a grabbable object
    * is removed. The connection created between the signal and the slot is
    * returned to the caller. It is up to the caller to manage the connection.
    *
    * @post \p slot is connected to the object removal signal.
    *
    * @param slot The slot to be connected to the object removal signal.
    *
    * @return The object holding the connection between the object removal
    *         signal and \p slot.
    *
    * @see removeObject()
    *
    * @since 0.23.1
    */
   boost::signals::connection connectToRemove(signal_t::slot_type slot);

   /**
    * Adds the given object to the list of grabbable objects and emits the
    * object addition signal.
    *
    * @post \p obj is in \c mObjects. Slots connected to the object addition
    *       signal are notified of the change.
    *
    * @param obj The scene object to add.
    */
   void addObject(SceneObjectPtr obj);

   /**
    * Removes the given object from the list of grabbable objects. If \p obj
    * is not currently in the list of grabbable objects, this method has no
    * effect. If \p obj is removed from the list of grabbable object, the
    * object removal signal is emitted.
    *
    * @post \p obj is not in \c mObjects. Slots connected to the object
    *       removal signal are notified of the change (if \p obj is removed).
    *
    * @param obj The scene object to remove.
    */
   void removeObject(SceneObjectPtr obj);

   /**
    * Returns the current collection of grabbable objects.
    */
   const object_list_t& getObjects() const
   {
      return mObjects;
   }

protected:
   GrabData();

private:
   object_list_t mObjects;

   /** @name Signals */
   //@{
   signal_t mAddSignal;         /**< Object addition signal. */
   signal_t mRemoveSignal;      /**< Object removal signal. */
   //@}
};

}


#endif
