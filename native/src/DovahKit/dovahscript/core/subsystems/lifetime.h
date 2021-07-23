#pragma once
#include <QButtonGroup>
#include <QDialog>
#include <mutex>
#include "../../../helpers/singleton.h"

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
         // Holder for classes that don't directly belong to a widget hierarchy. This doesn't mean 
         // that they can't influence the lifetime of a hierarchy object; only that they cannot be 
         // directly traversed to from a hierarchy, nor traversed from to a hierarchy.
         template<typename T> struct _non_hierarchy_object_lists {
            std::mutex pd_mutex; // the (extant) list should not be touched from the non-script thread
            QVector<T*> extant;
            QVector<T*> pending_deletion;
         };

      protected:
         _non_hierarchy_object_lists<CanvasWidgetLayerData> canvas_layer_data;
         _non_hierarchy_object_lists<model_observer_t>      model_observers;
         struct {
            QVector<QDialog*> windows;
            struct {
               QVector<QWidget*>      widgets;
               QVector<QButtonGroup*> button_groups;
            } orphans;
            struct {
               QVector<QWidget*>      widgets;
               QVector<QButtonGroup*> button_groups;
            } pending_deletion;
         } hierarchy_objects;

         // Maps of object pointers to refcounts.
         struct {
            std::mutex lock;

            std::unordered_map<model_observer_t*, int> model_observers;
            std::unordered_map<QObject*, int> objects;
         } task_referenced_objects;

         int extant_widget_count = 0; // includes windows

      public:
         std::vector<QDialog*> get_script_windows();

         #pragma region Script thread functions
            QVector<model_observer_t*> get_extant_model_observers() const noexcept;
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
