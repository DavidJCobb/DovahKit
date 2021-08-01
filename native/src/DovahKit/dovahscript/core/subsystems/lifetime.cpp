#include "lifetime.h"
#include "../../../helpers/unordered_map.h"
#include "../../../ui/generic/CanvasWidget.h"
#include "events.h"
#include "userdata.h"
#include "../verify_threading.h"

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
      this->pending_lifetime_checks.main_thread_handler(impl::lifetime_check_queue::subsystem_passkey());
   }

   void lifetime::on_script_teardown() {
      auto& events_s = events::get();
      //
      this->pending_lifetime_checks.on_script_teardown(impl::lifetime_check_queue::subsystem_passkey());
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
         auto& list = this->hierarchy_objects.button_groups;
         for (auto* e : list) {
            events_s.abandon_object(*e);
            e->deleteLater();
         }
         list.clear();
      }
      {
         auto& list = this->hierarchy_objects.model_observers;
         for (auto* e : list)
            delete e;
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
   }

   void lifetime::on_hierarchy_bridge_severed(QObject* basis, QObject* severed_from) {
      require_client_thread();
      //
      this->pending_lifetime_checks.queue_check(*basis);
      this->pending_lifetime_checks.queue_check(*severed_from);
   }
   void lifetime::on_hierarchy_item_parent_changed(QObject* child, QObject* prior_parent) {
      require_client_thread();
      //
      auto* after_parent = child->parent();
      if (prior_parent) {
         this->pending_lifetime_checks.queue_check(*prior_parent);
      }
      if (after_parent && !prior_parent) {
         auto  guard = std::unique_lock(this->object_read_write_lock);
         auto& base  = this->hierarchy_objects.orphans;
         //
         if (auto* widget = qobject_cast<QWidget*>(child)) {
            auto& list = base.widgets;
            auto  i    = list.indexOf(widget);
            assert(i >= 0);
            list.remove(i);
         } else if (auto* cwe = qobject_cast<CanvasWidgetEntity*>(child)) {
            auto& list = base.canvas_widget_entities;
            auto  i    = list.indexOf(cwe);
            assert(i >= 0);
            list.remove(i);
         } else {
            // Don't bother handling button groups; we deal with those elsewhere
            assert(false && "lifetime::on_hierarchy_item_parent_changed called with unexpected hierarchy item type");
         }
      } else if (!after_parent && prior_parent) {
         auto  guard = std::unique_lock(this->object_read_write_lock);
         auto& base  = this->hierarchy_objects.orphans;
         //
         if (auto* widget = qobject_cast<QWidget*>(child)) {
            auto& list = base.widgets;
            assert(!list.contains(widget));
            list.push_back(widget);
         } else if (auto* cwe = qobject_cast<CanvasWidgetEntity*>(child)) {
            auto& list = base.canvas_widget_entities;
            assert(!list.contains(cwe));
            list.push_back(cwe);
         } else {
            // Don't bother handling button groups; we deal with those elsewhere
            assert(false && "lifetime::on_hierarchy_item_parent_changed called with unexpected hierarchy item type");
         }
      }
   }
   void lifetime::on_window_hidden(QDialog* window) {
      require_client_thread();
      //
      this->pending_lifetime_checks.queue_check(*window);
   }
   void lifetime::on_canvas_widget_layer_data_detached(CanvasWidgetLayerData* data) {
      require_client_thread();
      //
      this->pending_lifetime_checks.queue_check(*data);
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
   void lifetime::destroy_hierarchy_object(passkey_to<impl::hierarchy_crawler>, QObject& target) {
      require_client_thread();
      //
      if constexpr (debug_qobject_lifetimes) {
         qDebug("Destroying native object: %p (%s)", &target, _debug_get_object_classname(&target));
      }
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
         bool removed = this->hierarchy_objects.button_groups.removeOne(c);
         if constexpr (debug_qobject_lifetimes) {
            if (!removed)
               qDebug("Warning: QButtonGroup under destruction is not in the list of button groups: %p (%s)", &target, _debug_get_object_classname(&target));
         }
      } else if (auto* c = qobject_cast<CanvasWidgetEntity*>(&target)) {
         _remove_from_orphans(this->hierarchy_objects.orphans.canvas_widget_entities, c);
      } else {
         assert(false && "unhandled object non-widget type");
      }
      //
      // Disconnect events:
      //
      events::get().abandon_object(target);
      //
      target.deleteLater();
   }

   bool lifetime::set_task_reference_lock_state(passkey_to<impl::task_reference_state_multi_checker>, bool state) {
      require_client_thread();
      //
      auto& lock = this->task_referenced_objects.lock;
      if (state)
         lock.lock();
      else
         lock.unlock();
   }
   bool lifetime::lockless_test_is_task_referenced(passkey_to<impl::task_reference_state_multi_checker>, QObject& subject) const noexcept {
      require_client_thread();
      //
      auto& tro = this->task_referenced_objects;
      return cobb::unordered_map_contains(tro.objects, &subject);
   }
   bool lifetime::lockless_test_is_task_referenced(passkey_to<impl::task_reference_state_multi_checker>, model_observer_t& subject) const noexcept {
      require_client_thread();
      //
      auto& tro = this->task_referenced_objects;
      return cobb::unordered_map_contains(tro.model_observers, &subject);
   }

   void lifetime::decrease_extant_widget_count(passkey_to<impl::lifetime_check_queue>, unsigned int by) {
      require_client_thread();
      this->extant_widget_count -= by;
   }
}