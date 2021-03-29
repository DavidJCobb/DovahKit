#include "event.h"
#include "../editor_script_core.h"

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
      decltype(this->list) local;
      {
         auto guard = std::lock_guard(this->lock);
         std::swap(local, this->list);
      }
      auto& intfc = DovahKitScriptUIListenerInterface::get();
      for (auto* event : local) {
         intfc.fire_event(event->widget, event->event_name.c_str(), event->listener_name.c_str(), event->params);
         delete event;
      }
   }
   void ui_event_queue::push_back(ui_event* e) {
      auto guard = std::lock_guard(this->lock);
      this->list.push_back(e);
   }
}