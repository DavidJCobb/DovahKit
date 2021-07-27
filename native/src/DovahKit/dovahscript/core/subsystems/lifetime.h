#pragma once
#include <QButtonGroup>
#include <QDialog>
#include <mutex>
#include "../../../helpers/singleton.h"
#include "coordinator/client_thread_script_borrow_handle.h"

class  CanvasWidgetEntity;
class  CanvasWidgetLayerData;
struct ObservableStandardItemModelObserver;

namespace dovahscript::core::subsystems {
   class lifetime : cobb::singleton {
      public:
         inline static lifetime& get() {
            static lifetime instance;
            return instance;
         }

         using model_observer_t = ObservableStandardItemModelObserver;

      protected:
         struct {
            QVector<QDialog*> windows;
            struct {
               QVector<QWidget*>            widgets;
               QVector<QButtonGroup*>       button_groups;
               QVector<CanvasWidgetEntity*> canvas_widget_entities;
            } orphans;
         } hierarchy_objects;
         struct {
            QVector<CanvasWidgetLayerData*> canvas_layer_data;
            QVector<model_observer_t>       model_observers;
         } non_hierarchy_objects;

         // Maps of object pointers to refcounts.
         struct {
            std::mutex lock;

            std::unordered_map<model_observer_t*, int> model_observers;
            std::unordered_map<QObject*, int> objects;
         } task_referenced_objects;

         int extant_widget_count = 0; // includes windows

         struct {
            std::mutex lock;
            struct {
               std::vector<model_observer_t*> model_observers;
               std::vector<QObject*> objects;
            } queues;
            client_thread_script_borrow_handle opportunity_handle;
         } pending_lifetime_checks;

      public:
         std::vector<QDialog*> get_script_windows();

         // This should delete any pending-deletion model observers.
         void main_thread_handler();

         void on_script_teardown();

         #pragma region Script thread functions
            QVector<model_observer_t*> get_extant_model_observers() const noexcept;

            void on_lua_unreferenced(CanvasWidgetLayerData*);
            void on_lua_unreferenced(model_observer_t*);
            void on_lua_unreferenced(QObject*);
         #pragma endregion

         #pragma region Client thread functions
            void on_hierarchy_bridge_severed(QObject* basis, QObject* severed_from); // e.g. if a QButtonGroup loses a button, the group would be the basis and the button, the severed-from object
            void on_hierarchy_item_orphaned(QObject*);
            void on_hierarchy_item_adopted(QObject*);

            void on_window_hidden(QDialog*);

            void on_canvas_widget_layer_data_detached(CanvasWidgetLayerData*);
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
