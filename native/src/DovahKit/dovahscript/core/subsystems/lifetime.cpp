#include "lifetime.h"
#include "events.h"
#include "../../../ui/generic/CanvasWidget.h"

namespace {
   #pragma region Logging options
   static constexpr bool debug_model_observer_lifetimes = false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;

   static constexpr bool debug_qobject_lifetimes = false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;
   #pragma endregion

   const char* _debug_get_object_classname(const QObject* o) {
      if (!o)
         return "nullptr";
      auto* mo = o->metaObject();
      if (!mo)
         return "unknown type";
      auto* name = mo->className();
      if (!name)
         return "unknown type";
      return name;
   }
}

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

   namespace {
      template<typename T> void _remove_from_orphans(QVector<T*>& list, T* target) {
         auto  i = list.indexOf((QWidget*)&target);
         if (i >= 0) {
            list.remove(i);
         } else {
            if constexpr (debug_qobject_lifetimes) {
               qDebug("Warning: QObject under destruction is not orphaned: %p (%s)", &target, _debug_get_object_classname(target));
            }
         }
      }
   }
   void lifetime::destroy_native_object(passkey_to<impl::hierarchy_finder>, QObject& target) {
      if (target.isWidgetType()) {
         _remove_from_orphans(this->hierarchy_objects.orphans.widgets, (QWidget*)&target);
         //
         if (auto* window = qobject_cast<QDialog*>(&target)) {
            auto& list = this->hierarchy_objects.windows;
            auto  i    = list.indexOf(window);
            if (i >= 0) {
               list.remove(i);
            } else {
               if constexpr (debug_qobject_lifetimes) {
                  qDebug("Warning: QDialog under destruction is not in the list of windows: %p (%s)", &target, _debug_get_object_classname(&target));
               }
            }
         }
      } else if (auto* c = qobject_cast<QButtonGroup*>(&target)) {
         _remove_from_orphans(this->hierarchy_objects.orphans.button_groups, c);
      } else if (auto* c = qobject_cast<CanvasWidgetEntity*>(&target)) {
         _remove_from_orphans(this->hierarchy_objects.orphans.canvas_widget_entities, c);
      } else {
         assert(false && "unhandled object non-widget type");
      }
      //
      // Disconnect events:
      //
      events::get().abandon_object(target);
   }
}