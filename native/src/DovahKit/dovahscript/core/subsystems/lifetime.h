#pragma once
#include <QButtonGroup>
#include <QDialog>
#include <mutex>
#include <shared_mutex>
#include "../../../helpers/passkey.h"
#include "../../../helpers/singleton.h"
#include "coordinator/client_thread_script_borrow_handle.h"
#include "lifetime/lifetime_check_queue.h"

class  CanvasWidgetEntity;
class  CanvasWidgetLayerData;
struct ObservableStandardItemModelObserver;

namespace dovahscript::impl {
   class hierarchy_crawler;
   class task_reference_state_multi_checker;
}
namespace dovahscript::core::subsystems {
   class userdata;
}

namespace dovahscript::core::subsystems {
   class lifetime : cobb::singleton {
      public:
         inline static lifetime& get() {
            static lifetime instance;
            return instance;
         }

         using model_observer_t = ObservableStandardItemModelObserver;

         template<typename B> using passkey_to = cobb::passkey<lifetime, B>;

      protected:

         //
         // These are the objects whose lifetimes are managed directly by this singleton. 
         // These lists should only be modified from the client thread, but should allow 
         // reading from any thread. Accordingly, we use a shared_mutex, with the client 
         // thread taking an exclusive lock and the worker thread taking a shared lock.
         //
         std::shared_mutex object_read_write_lock;
         struct {
            QVector<QDialog*>          windows;
            QVector<QButtonGroup*>     button_groups;
            QVector<model_observer_t*> model_observers;
            struct {
               QVector<QWidget*>            widgets;
               QVector<CanvasWidgetEntity*> canvas_widget_entities;
            } orphans;
         } hierarchy_objects;
         struct {
            QVector<CanvasWidgetLayerData*> canvas_layer_data;
         } non_hierarchy_objects;

         // Maps of object pointers to refcounts.
         struct {
            std::mutex lock;

            std::unordered_map<model_observer_t*, int> model_observers;
            std::unordered_map<QObject*, int> objects;
         } task_referenced_objects;

         int extant_widget_count = 0; // includes windows

         impl::lifetime_check_queue pending_lifetime_checks;

      public:
         std::vector<QDialog*> get_script_windows();

         // This should delete any pending-deletion model observers.
         void main_thread_handler();

         void on_script_teardown();

         #pragma region Script thread functions
            QVector<model_observer_t*> get_extant_model_observers() const noexcept;

            void on_lua_unreferenced(passkey_to<userdata>, CanvasWidgetLayerData*);
            void on_lua_unreferenced(passkey_to<userdata>, model_observer_t*);
            void on_lua_unreferenced(passkey_to<userdata>, QObject*);
         #pragma endregion

         #pragma region Client thread functions
            void on_hierarchy_bridge_severed(QObject* basis, QObject* severed_from); // e.g. if a QButtonGroup loses a button, the group would be the basis and the button, the severed-from object
            void on_hierarchy_item_parent_changed(QObject* subject, QObject* prior_parent); // call from the client thread after the subject's parent has been changed

            void on_window_hidden(QDialog*);

            void on_canvas_widget_layer_data_detached(CanvasWidgetLayerData*);

            void for_each_known_model_observer(std::function<void(model_observer_t*)>);

            // The hierarchy finder should call these if it confirms that a native object is unreferenced 
            // and unreachable.
            void destroy_hierarchy_object(passkey_to<impl::hierarchy_crawler>, QObject&);
            void destroy_hierarchy_object(passkey_to<impl::hierarchy_crawler>, model_observer_t&);

            void destroy_non_hierarchy_object(passkey_to<impl::lifetime_check_queue>, QObject&);

            // These functions are used during processing of the pending lifetime check queue, to quickly 
            // check whether a given native object is task-referenced. We allow the queue to grab the lock 
            // and hold it using an RAII struct, to avoid having to constantly lock and unlock for each 
            // individual check.
            bool set_task_reference_lock_state(passkey_to<impl::task_reference_state_multi_checker>, bool);
            bool lockless_test_is_task_referenced(passkey_to<impl::task_reference_state_multi_checker>, QObject&) const noexcept;
            bool lockless_test_is_task_referenced(passkey_to<impl::task_reference_state_multi_checker>, model_observer_t&) const noexcept;

            void decrease_extant_widget_count(passkey_to<impl::lifetime_check_queue>, unsigned int);
         #pragma endregion

         // When the script thread queues a cross-thread task that in some way uses or refers to a 
         // QObject or a model observer, the task must notify the lifetime subsystem and mark the 
         // referred-to object as "task-referenced." Of course, objects can be referenced by more 
         // than one task, so we store a refcount internally.
         void add_task_reference(QObject*);
         void add_task_reference(model_observer_t*);

         // When the client thread executes a cross-thread task that in some way uses or refers to 
         // a QObject or a model observer, the task must notify the lifetime subsystem and mark the 
         // referred-to object as "task-unreferenced."
         // 
         // Of course, some tasks aren't simply fire-and-forget, but rather return results to their 
         // sender; these tasks should mark the referred-to object as "task-unreferenced" not from 
         // the client thread, but from the script thread. Examples include tasks which create a 
         // new UI widget; these tasks should mark the widget as task-unreferenced only when they 
         // are destroyed, and in turn, the APIs which destroy them should do so only after they 
         // have wrapped the widgets in userdata and pushed that userdata into Lua.
         void remove_task_reference(QObject*);
         void remove_task_reference(model_observer_t*);
   };
}
