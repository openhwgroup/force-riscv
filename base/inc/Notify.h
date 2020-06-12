//
// Copyright (C) [2020] Futurewei Technologies, Inc.
//
// FORCE-RISCV is licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
// FIT FOR A PARTICULAR PURPOSE.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef Force_Notify_H
#define Force_Notify_H

#include <UtilityAlgorithms.h>
#include <Log.h>
#include <string>
#include <vector>

namespace Force {

  class Object;

  template <typename EventType>
  class Receiver;

  template <typename EventType>
  class Sender;

  /*!
    \struct NotificationTuple
    \brief A tuple style struct containing notification Sender object and event type.
  */
  template <typename EventType>
  struct NotificationTuple {
    static_assert(std::is_enum<EventType>::value, "EventType must be an enum.");

    const Sender<EventType>* mpSender; //!< Pointer to the Sender<EventType> object.
    EventType mNotification; //!< Notification type.
    Object* mpPayload; //!< Payload data object.
  public:
    NotificationTuple(const Sender<EventType>* sender, EventType eventType, Object* pPayload) : mpSender(sender), mNotification(eventType), mpPayload(pPayload) { } //!< Constructor.
    NotificationTuple() : mpSender(nullptr), mNotification(EventType(0)), mpPayload(nullptr) { } //!< Default constructor.

    inline bool operator == (const NotificationTuple<EventType>& rOther) const //!< Equal comparing operator.
    {
      if (mpSender != rOther.mpSender) return false;
      if (mNotification != rOther.mNotification) return false;
      return (mpPayload == rOther.mpPayload);
    }

    inline bool operator < (const NotificationTuple<EventType>& rOther) const //!< Less than comparing operator.
    {
      if (mpSender < rOther.mpSender) return true;
      if (mpSender > rOther.mpSender) return false;
      if (mNotification < rOther.mNotification) return true;
      if (mNotification > rOther.mNotification) return false;
      return (mpPayload < rOther.mpPayload);
    }

  };

  /*!
    \class Sender
    \brief A simple class sending out notifications to Receiver objects
   */
  template <typename EventType>
  class Sender {
    static_assert(std::is_enum<EventType>::value, "EventType must be an enum.");

  public:
    Sender() : mReceivers(), mCachedNotifications(), mBlockSend(false) { } //!< Default constructor.

    Sender(const Sender<EventType>& rOther) : mReceivers(), mCachedNotifications(), mBlockSend(false) { } //!< Copy consructor.

    //!< Destructor.
    virtual ~Sender()
    {
      if (mBlockSend) {
        LOG(warn) << "Sender still blocked when destructed." << std::endl;
      } else if (mCachedNotifications.size() > 0) {
        LOG(fail) << "Sender still has cached notifications when destructed." << std::endl;
        FAIL("sender-cache-not-clear");
      }
    }

    void SignUp(Receiver<EventType>* receiver) const { insert_sorted<Receiver<EventType>*>(mReceivers, receiver); } //!< Sign up with the Sender object to receive event notification.

    //!< Called to send notification to receivers, with optional payload data.
    void SendNotification(EventType eventType, Object* pData = nullptr) const
    {
      if (mBlockSend) {
        CacheNotification(eventType, pData);
        return;
      }

      SendNotificationNoBlocking(eventType, pData);
    }

    void Block() { mBlockSend = true; } //!< Called to block the Sender object from sending out notification.

    //!< Called to unblock the Sender object, and send out cached notifications.
    void Unblock()
    {
      uint32 loop_count = 0;
      while (mCachedNotifications.size() > 0) {
        // need the while loop since the notification could trigger no events cached into the Sender object.
        auto cached_copy = mCachedNotifications; // make a copy of the event list.
        mCachedNotifications.clear();

        for (auto notif_item : cached_copy) {
          SendNotificationNoBlocking(notif_item.mNotification, notif_item.mpPayload);
        }

        ++ loop_count;
        if (loop_count > 2) {
          LOG(fail) << "{Sender::Unblock} not expecting the unblocking loop to loop more than 2 times." << std::endl;
          FAIL("unblock-sender-loop-too-many-times");
        }
      }

      // now we can unblock.
      mBlockSend = false;
    }

    //!< Called to unblock the Sender object, and clear the cached notifications.
    void Clear()
    {
      mCachedNotifications.clear();
      mBlockSend = false;
    }

    virtual const std::string Description() const { return ""; } //!< Return a description string for the Sender object.
  private:
    //!< Called to send notification to receivers, no blocking.
    void SendNotificationNoBlocking(EventType eventType, Object* pData) const
    {
      for (auto receiver_item : mReceivers) {
        receiver_item->Notify(this, eventType, pData);
      }
    }

    //!< Cache a notification.
    void CacheNotification(EventType eventType, Object* pData) const
    {
      NotificationTuple<EventType> notif_tuple(this, eventType, pData);
      insert_sorted<NotificationTuple<EventType>>(mCachedNotifications, notif_tuple);
    }
  private:
    mutable std::vector<Receiver<EventType>*> mReceivers; //!< Registered Receiver objects.
    mutable std::vector<NotificationTuple<EventType>> mCachedNotifications; //!< Notifications that are cached.
    bool mBlockSend; //!< Block sending notification.
  };

  /*!
    \class Receiver
    \brief A simple class receiving notifications from Sender objects.
  */
  template <typename EventType>
  class Receiver {
    static_assert(std::is_enum<EventType>::value, "EventType must be an enum.");

  public:
    Receiver() : mCachedEvents(), mBlockReceive(false) { } //!< Default constructor.

    Receiver(const Receiver<EventType>& rOther) : mCachedEvents(), mBlockReceive(false) { } //!< Copy constructor.

    //!< Destructor.
    virtual ~Receiver()
    {
      if (mBlockReceive) {
        LOG(warn) << "Receiver still blocked when destructed." << std::endl;
      } else if (mCachedEvents.size() > 0) {
        LOG(fail) << "Receiver still has cached notifications when destructed." << std::endl;
        FAIL("receiver-cache-not-clear");
      }
    }

    //!< Called to notify the Receiver object regarding an event.
    void Notify(const Sender<EventType>* sender, EventType eventType, Object* pPayload)
    {
      if (mBlockReceive) {
        CacheEvent(sender, eventType, pPayload);
        return;
      }

      HandleNotification(sender, eventType, pPayload);
    }

    void Block() { mBlockReceive = true; } //!< Called to block the Receiver object from reacting to notifications.

    //!< Called to unblock the Receiver object, and process cached notifications.
    void Unblock()
    {
      uint32 loop_count = 0;
      while (mCachedEvents.size() > 0) {
        // need the while loop since the notification could trigger no events cached into the Sender object.
        auto cached_copy = mCachedEvents; // make a copy of the event list.
        mCachedEvents.clear();

        for (auto notif_tuple : cached_copy) {
          HandleNotification(notif_tuple.mpSender, notif_tuple.mNotification, notif_tuple.mpPayload);
        }

        ++ loop_count;
        if (loop_count > 2) {
          LOG(fail) << "{Receiver::Unblock} not expecting the unblocking loop to loop more than 2 times." << std::endl;
          FAIL("unblock-receiver-loop-too-many-times");
        }
      }

      // now we can unblock.
      mBlockReceive = false;
    }

    //!< Called to unblock the Receiver object, and clear the cached notifications.
    void Clear()
    {
      mCachedEvents.clear();
      mBlockReceive = false;
    }

    virtual const std::string Description() const { return ""; } //!< Return a description string for the Sender object.
  protected:
    virtual void HandleNotification(const Sender<EventType>* sender, EventType eventType, Object* pPayload) = 0; //!< Handle a notification.
  private:
    //!< Cache event when receiving is blocked.
    void CacheEvent(const Sender<EventType>* sender, EventType eventType, Object* pPayload)
    {
      NotificationTuple<EventType> notif_event(sender, eventType, pPayload);
      insert_sorted<NotificationTuple<EventType>>(mCachedEvents, notif_event);
    }
  private:
    std::vector<NotificationTuple<EventType>> mCachedEvents; //!< Event notification that are cached.
    bool mBlockReceive; //!< Block receiving notification.
  };

}

#endif
