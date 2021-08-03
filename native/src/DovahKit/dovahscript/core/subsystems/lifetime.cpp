#include "lifetime.h"
#include "../../../helpers/unordered_map.h"
#include "../../../ui/generic/CanvasWidget.h"
#include "coordinator.h"
#include "events.h"
#include "userdata.h"
#include "../verify_threading.h"
#include "../../qt/DovahscriptDialog.h"

namespace {

   //
   // There are two ways we can handle objects becoming task-unreferenced. The first is to 
   // queue lifetime checks immediately upon their becoming task-unreferenced. The problem 
   // is that many if not most task-unreferenced objects will remain Lua-referenced for at 
   // least a little while, so an lifetime check will end up being a waste of effort; in 
   // fact, it would largely negate many of the advantages of using non-blocking tasks.
   // 
   // The other option is to have the script thread scan for objects that are in the task-
   // referenced object list, but have a refcount of zero -- so, objects that have become 
   // task-unreferenced, and that haven't been tended to yet. The script thread can then 
   // check if these objects are Lua-unreferenced, and queue a lifetime check only if they 
   // are. There's still some overhead here, but we're not doing a full lock-and-synch 
   // between both threads.
   // 
   // Either way, once an object becomes task-unreferenced and that fact has been attended 
   // to, its (zero) count needs to be removed from the task-referenced list.
   //
   static constexpr bool notify_for_task_references = false;

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
   void lifetime::on_script_setup() {
      assert(this->hierarchy_objects.windows.empty());
      assert(this->hierarchy_objects.button_groups.empty());
      assert(this->hierarchy_objects.model_observers.empty());
      assert(this->hierarchy_objects.orphans.canvas_widget_entities.empty());
      assert(this->hierarchy_objects.orphans.widgets.empty());
      assert(this->non_hierarchy_objects.canvas_layer_data.empty());
      {
         auto& tro   = this->task_referenced_objects;
         auto  guard = std::lock_guard(tro.lock);
         assert(tro.model_observers.empty());
         assert(tro.objects.empty());
      }
      assert(this->extant_widget_count == 0);
      assert(this->pending_lifetime_checks.empty());
   }

   void lifetime::main_thread_handler() {
      this->pending_lifetime_checks.main_thread_handler(impl::lifetime_check_queue::subsystem_passkey());
   }

   void lifetime::worker_thread_handler() {
      if constexpr (!notify_for_task_references) {
         if (coordinator::get().script_thread == thread_type::worker) {
            //
            // We've been configured so that instead of immediately acting on objects becoming 
            // task-unreferenced (inevitably on the client thread) by queuing a full lifetime 
            // check for them, we instead want to have the worker thread passively scan for 
            // objects that have just become task-unreferenced. This will give us a chance to 
            // quickly check whether the objects are Lua-referenced, and only queue lifetime 
            // checks for those that aren't -- allowing us to potentially avoid the overhead 
            // of fully synchronizing both of our threads for a full lifetime check.
            //
            auto& userdata_s = userdata::get();
            //
            auto& tro   = this->task_referenced_objects;
            auto  guard = std::lock_guard(tro.lock);
            {
               auto& um = tro.objects;
               auto  it = um.begin();
               while (it != um.end()) {
                  auto& pair = *it;
                  auto* obj  = pair.first;
                  if (pair.second == 0) {
                     if (!userdata_s.wrapper_exists_for(obj))
                        this->pending_lifetime_checks.queue_check(*obj);
                     //
                     // The return value of std::unordered_map::erase can be used to avoid 
                     // having iterator invalidation break a loop. However, you must be 
                     // sure not to increment the iterator after grabbing this return value 
                     // (as you typically would during a for-loop).
                     //
                     it = um.erase(it);
                  } else {
                     ++it;
                  }
               }
            }
            {
               auto& um = tro.model_observers;
               auto  it = um.begin();
               while (it != um.end()) {
                  auto& pair = *it;
                  auto* obj  = pair.first;
                  if (pair.second == 0) {
                     if (!userdata_s.wrapper_exists_for(obj))
                        this->pending_lifetime_checks.queue_check(*obj);
                     //
                     // The return value of std::unordered_map::erase can be used to avoid 
                     // having iterator invalidation break a loop. However, you must be 
                     // sure not to increment the iterator after grabbing this return value 
                     // (as you typically would during a for-loop).
                     //
                     it = um.erase(it);
                  } else {
                     ++it;
                  }
               }
            }
         }
      }
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


   QVector<DovahscriptDialog*> lifetime::get_script_windows() const noexcept {
      auto guard = std::shared_lock(this->object_read_write_lock);
      return this->hierarchy_objects.windows;
   }

   bool lifetime::any_windows_visible_or_task_referenced() const noexcept {
      auto  guard = std::shared_lock(this->object_read_write_lock);
      auto& tro   = this->task_referenced_objects;
      auto  tro_g = std::lock_guard(tro.lock);
      for (auto* window : this->hierarchy_objects.windows) {
         //
         // Is the window visible?
         //
         if (window->lastVisibleState()) // QWidget::isVisible is not atomic or otherwise thread-safe; this should be
            return true;
         //
         // Is the window task-referenced?
         //
         auto it = tro.objects.find(window);
         if (it != tro.objects.end())
            if (it->second > 0)
               return true;
      }
      return false;
   }


   void lifetime::on_lua_unreferenced(passkey_to<userdata>, CanvasWidgetLayerData* cwld) {
      require_script_thread();
      //
      this->pending_lifetime_checks.queue_check(*cwld);
   }
   void lifetime::on_lua_unreferenced(passkey_to<userdata>, model_observer_t* observer) {
      require_script_thread();
      //
      this->pending_lifetime_checks.queue_check(*observer);
   }
   void lifetime::on_lua_unreferenced(passkey_to<userdata>, QObject* object) {
      require_script_thread();
      //
      this->pending_lifetime_checks.queue_check(*object);
   }


   void lifetime::on_hierarchy_item_created(QObject& object) {
      require_client_thread();
      //
      auto guard = std::unique_lock(this->object_read_write_lock);
      if (object.isWidgetType()) {
         auto* widget = (QWidget*)&object;
         ++this->extant_widget_count;
         if (auto* dialog = qobject_cast<QDialog*>(&object)) {
            this->hierarchy_objects.windows.push_back(dialog);
         } else {
            if (!object.parent())
               this->hierarchy_objects.orphans.widgets.push_back(widget);
         }
      } else {
         if (auto* bg = qobject_cast<QButtonGroup*>(&object)) {
            this->hierarchy_objects.button_groups.push_back(bg);
         } else if (auto* cwld = qobject_cast<CanvasWidgetLayerData*>(&object)) {
            this->non_hierarchy_objects.canvas_layer_data.push_back(cwld);
         }
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
            assert(false && "lifetime::on_hierarchy_item_parent_changed called with unexpected hierarchy item type (item adopted)");
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
            assert(false && "lifetime::on_hierarchy_item_parent_changed called with unexpected hierarchy item type (item orphaned)");
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


   void lifetime::for_each_known_model_observer(std::function<void(model_observer_t*)> functor) {
      require_client_thread();
      //
      auto guard = std::shared_lock(this->object_read_write_lock);
      for (auto* observer : this->hierarchy_objects.model_observers)
         (functor)(observer);
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
         qDebug("Destroying native hierarchy object: %p (%s)", &target, _debug_get_object_classname(&target));
      }
      {
         auto guard = std::unique_lock(this->object_read_write_lock);
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
      }
      //
      // Disconnect events:
      //
      events::get().abandon_object(target);
      //
      target.deleteLater();
   }
   void lifetime::destroy_hierarchy_object(passkey_to<impl::hierarchy_crawler>, model_observer_t& target) {
      require_client_thread();
      //
      if constexpr (debug_model_observer_lifetimes) {
         qDebug("Destroying native hierarchy model observer: %p", &target);
      }
      auto  guard = std::unique_lock(this->object_read_write_lock);
      auto& list  = this->hierarchy_objects.model_observers;
      auto  i     = list.indexOf(&target);
      if (i >= 0) {
         list.remove(i);
      } else {
         if constexpr (debug_model_observer_lifetimes) {
            qDebug("Warning: model observer under destruction is not orphaned: %p", &target);
         }
      }
   }
   void lifetime::destroy_non_hierarchy_object(passkey_to<impl::lifetime_check_queue>, QObject& target) {
      require_client_thread();
      //
      if constexpr (debug_qobject_lifetimes) {
         qDebug("Destroying native non-hierarchy object: %p (%s)", &target, _debug_get_object_classname(&target));
      }
      auto guard = std::unique_lock(this->object_read_write_lock);
      if (auto* cwld = qobject_cast<CanvasWidgetLayerData*>(&target)) {
         auto& list = this->non_hierarchy_objects.canvas_layer_data;
         auto  i    = list.indexOf(cwld);
         if (i >= 0) {
            list.remove(i);
         } else {
            if constexpr (debug_qobject_lifetimes) {
               qDebug("Warning: QObject under destruction is not orphaned: %p (%s)", cwld, _debug_get_object_classname(cwld));
            }
         }
         events::get().abandon_object(target); // CWLDs shouldn't have any events, but let's not risk forgetting this if we ever have reason to add any
         return;
      }
      assert(false && "unhandled type");
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

   void lifetime::add_task_reference(QObject* subject) {
      if (!subject)
         return;
      auto& tro   = this->task_referenced_objects;
      auto  guard = std::lock_guard(tro.lock);
      ++tro.objects[subject];
   }
   void lifetime::add_task_reference(model_observer_t* subject) {
      if (!subject)
         return;
      auto& tro   = this->task_referenced_objects;
      auto  guard = std::lock_guard(tro.lock);
      ++tro.model_observers[subject];
   }

   void lifetime::remove_task_reference(QObject* subject) {
      if (!subject)
         return;
      auto& tro   = this->task_referenced_objects;
      auto  guard = std::lock_guard(tro.lock);
      auto& count = tro.objects[subject];
      assert(count);
      if constexpr (notify_for_task_references) {
         if (--count == 0) {
            tro.objects.erase(subject);
            this->pending_lifetime_checks.queue_check(*subject);
         }
      }
   }
   void lifetime::remove_task_reference(model_observer_t* subject) {
      if (!subject)
         return;
      auto& tro   = this->task_referenced_objects;
      auto  guard = std::lock_guard(tro.lock);
      auto& count = tro.model_observers[subject];
      assert(count);
      --count;
      if constexpr (notify_for_task_references) {
         if (count == 0) {
            tro.model_observers.erase(subject);
            this->pending_lifetime_checks.queue_check(*subject);
         }
      }
   }
}