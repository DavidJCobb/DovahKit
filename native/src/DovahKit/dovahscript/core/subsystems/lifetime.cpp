#include "lifetime.h"
#include "events.h"
#include "../../../ui/generic/CanvasWidget.h"

namespace dovahscript::core::subsystems {
   void lifetime::main_thread_handler() {
      {
         auto& plc   = this->pending_lifetime_checks;
         auto  guard = std::lock_guard(plc.lock);
         if (plc.opportunity_handle.is_ready()) {
            static_assert(false, "TODO: Perform lifetime checks on the queued objects.");
         } else {
            bool empty = plc.queues.objects.empty() && plc.queues.model_observers.empty();
            if (!empty) {
               plc.opportunity_handle.request();
            }
         }
      }

      static_assert(false, "TODO: Do we need anything else here?");
   }

   void lifetime::on_script_teardown() {
      auto& events_s = events::get();
      //
      {
         auto& plc   = this->pending_lifetime_checks;
         auto  guard = std::lock_guard(plc.lock);
         plc.queues.objects.clear();
         plc.queues.model_observers.clear();
         //
         plc.opportunity_handle.release();
      }
      {
         auto& tro   = this->task_referenced_objects;
         auto  guard = std::lock_guard(tro.lock);
         tro.model_observers.clear();
         tro.objects.clear();
      }
      {
         auto& list = this->hierarchy_objects.windows;
         for (auto* e : list) {
            events_s.abandon_object(*e);
            e->deleteLater();
         }
         list.clear();
      }
      {
         auto& list = this->hierarchy_objects.orphans.button_groups;
         for (auto* e : list) {
            events_s.abandon_object(*e);
            e->deleteLater();
         }
         list.clear();
      }
      {
         auto& list = this->hierarchy_objects.orphans.canvas_widget_entities;
         for (auto* e : list) {
            events_s.abandon_object(*e);
            e->deleteLater();
         }
         list.clear();
      }
      {
         auto& list = this->hierarchy_objects.orphans.widgets;
         for (auto* e : list) {
            events_s.abandon_object(*e);
            e->deleteLater();
         }
         list.clear();
      }
      this->extant_widget_count = 0;
      {
         auto& list = this->non_hierarchy_objects.canvas_layer_data;
         for (auto* e : list)
            e->deleteLater();
         list.clear();
      }
      {
         auto& list = this->non_hierarchy_objects.model_observers;
         for (auto* e : list)
            delete e;
         list.clear();
      }
      static_assert(false, "TODO: Do we need anything else here?");
   }
}