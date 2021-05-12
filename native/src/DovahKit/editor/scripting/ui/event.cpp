#include "event.h"
#include "../systems/ui_listeners.h"

namespace editor_script {
   void ui_event_queue::clear() {
      decltype(this->list) local;
      {
         auto guard = std::lock_guard(this->lock);
         std::swap(local, this->list);
      }
      for (auto* event : local)
         delete event;
   }
   void ui_event_queue::process() {
      //
      // We copy all pending events out of the cross-thread queue and into a local list, so that we 
      // can keep the cross-thread queue locked as briefly as possible. This was a pertinent concern 
      // during a previous design for UI locking, wherein it could become possible for the main thread 
      // to send an event to Lua while a listener for a previously-received event was already running; 
      // if the queue remained locked while executing already-received event, then the main thread 
      // would be blocked.
      //
      // As of this writing [3/31/2021], that sort of collision shouldn't be possible due to changes 
      // to how UI locking works. As such, this alternate way of executing the task queue isn't 
      // strictly needed, but there's also no specific reason to change it.
      //
      decltype(this->list) local;
      {
         auto guard = std::lock_guard(this->lock);
         std::swap(local, this->list);
      }
      auto& intfc = DovahKitScriptUIListenerInterface::get();
      for (auto* event : local) {
         intfc.fire_event(event->target, event->event_name.c_str(), event->listener_name.c_str(), event->params);
         delete event;
      }
   }
   void ui_event_queue::push_back(ui_event* e) {
      auto guard = std::lock_guard(this->lock);
      this->list.push_back(e);
   }

   void ui_event_queue::forget_about(QObject& target) {
      auto  guard = std::lock_guard(this->lock);
      auto& list  = this->list;
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [&target](const ui_event* entry) {
               return (&entry->target == &target);
            }
         ),
         list.end()
      );
   }
}