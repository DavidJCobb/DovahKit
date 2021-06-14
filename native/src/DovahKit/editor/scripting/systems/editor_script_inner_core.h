#pragma once
#include "../../../lua.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <QDialog>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QWidget>
#include "../../../helpers/lockable_bool.h"
#include "../../../helpers/singleton.h"

#include "../cross_thread_tasks/base.h"
#include "../ui/event.h"
#include "../ui/util/lua_item_model.h"
#include "../wrapper.h"

namespace dovah {
   class form_stub;
}

class CanvasWidgetLayerData;
class LuaScriptableCanvasWidgetLayerData;
class QButtonGroup;

class DovahKitScriptVMMessenger;
class DovahKitScriptVMUITaskConduit;
class DovahKitScriptVMPermissionInterface;
class DovahKitScriptVMResourceInterface;
class DovahKitScriptVMUserdataInterface;
class DovahKitScriptUIListenerInterface;

class DovahKitScriptVMCore : public QObject, cobb::singleton {
   Q_OBJECT
   //
   // This is the core singleton for editor scripting. It is intended to be accessed directly by 
   // the main thread, and will run Lua scripts on a secondary thread. The singleton has its own 
   // Qt signal for the main thread event loop, and  will loop in the secondary thread as needed 
   // to block script execution  while waiting for any needed  information from the main thread.
   //
   friend class DovahKitScriptVMMessenger;
   friend class DovahKitScriptVMUITaskConduit;
   friend class DovahKitScriptVMPermissionInterface;
   friend class DovahKitScriptVMResourceInterface;
   friend class DovahKitScriptVMUserdataInterface;
   friend class DovahKitScriptUIListenerInterface;
   using model_observer_t = ObservableStandardItemModelObserver;
   public:

      // Storage in the Lua registry for a cached copy of (string.format), which we place there 
      // when we start up the VM, to ensure that hardcoded functions that need that behavior can 
      // access it even if the Lua script tries to monkeypatch or replace its own copy.
      static constexpr const char* string_format_registry_key   = "cached:string.format";

      static constexpr const char* wrapper_storage_registry_key  = "dovah.internals.extant_wrappers";
      static constexpr const char* wrapper_weakmap_metatable_key = "__weakmap_mode_metatable";

      // Storage in the Lua registry for all Lua event listeners registered on a widget.
      static constexpr const char* ui_listener_registry_key = "dovah.internals.ui_listeners"; // registry[key][widget_pointer][event_name][listener_name]

      // Storage in the Lua registry for functions that have been queued by the script to execute 
      // when the UI is later locked or unlocked.
      static constexpr const char* ui_locked_queue_registry_key   = "dovah.internals.run_when_ui_locked_queue";
      static constexpr const char* ui_unlocked_queue_registry_key = "dovah.internals.run_when_ui_unlocked_queue";

      enum class ui_lock_override_state { unchanged, locked, unlocked };

   protected:
      DovahKitScriptVMCore();
      ~DovahKitScriptVMCore();

      struct _task_queue {
         using task = editor_script::cross_thread_task;
         //
         std::vector<task*> list;
         std::recursive_mutex lock;

         //
         // The receiving thread should use this function to execute tasks.
         //
         void process(int cap = std::numeric_limits<int>::max());

         //
         // Suitable only for use by the sending thread.
         //
         void wait_until_empty();

         void clear();
      };

      // This class is capable of traversing an entire set of bridged widget hierarchies -- that 
      // is, hierarchies of QWidgets which may be "bridged" to each other by objects such as a 
      // QButtonGroup. These objects are considered interdependent: if any object in a set of 
      // bridged hierarchies is Lua-referenced, then all of the objects are potentially reachable 
      // through Lua, and so none can be deleted.
      //
      // Used to determine when to delete objects that are no longer referenced or referenceable 
      // by Lua. Refer to our internal documentation on widget lifetimes for further information.
      class _hierarchy_finder {
         protected:
            struct search_results {
               int total_widget_count = 0; // count of all widgets in all found hierarchies
               QList<QWidget*>      widgets; // hierarchy root widgets
               QList<QButtonGroup*> button_groups;
               //
               void append(const search_results&) noexcept;
               void clear() noexcept;
               bool empty() const noexcept;
               bool has_hierarchy_bridges() const noexcept;
               
               bool contains(QButtonGroup*) const noexcept;
               bool contains(QWidget*) const noexcept;

               // Some of the object types that we search for, like QButtonGroup, can bridge multiple 
               // widget hierarchies together. This function returns a list of all root widgets of 
               // all hierarchies that this (search_results) instance's "bridges" connect to, unless 
               // the bridges themselves or the found roots are in the (ignore) instance.
               QList<QWidget*> get_linked_hierarchies(const search_results& ignore) const noexcept;
            };

            QVector<ObservableStandardItemModel*> referenced_models; // models known in advance not to be abandoned
            struct {
               bool halt_and_clear_upon_non_abandoned = true;
            } options;
            search_results results; // (total_widget_count) is 0 unless all hierarchies are abandoned

            // Given some "basis" widget, traverse the widget hierarchy that that widget belongs to, 
            // checking whether any widgets or "bridge" objects in that hierarchy are Lua-referenced. 
            // This function also returns a list of the "bridges," but does not cross them to examine 
            // other hierarchies; that's the caller's responsibility.
            // 
            // Returns false if it halts the search as per (options.halt_and_clear_upon_non_abandoned).
            bool _traverse_from_basis(QWidget* basis, search_results& out);

            // Given a basis widget, searches the widget's entire containing hierarchy as well as any 
            // containing hierarchies linked by a QButtonGroup. Returns false if it halts the search 
            // as per (options.halt_and_clear_upon_non_abandoned).
            bool _start_from_basis(QObject*);

         public:
            void submit_non_abandoned_model(ObservableStandardItemModel*) noexcept;
            void import_non_abandoned_models(DovahKitScriptVMCore&, ObservableStandardItemModelObserver* exclude = nullptr) noexcept;

            void gather_from(QObject*) noexcept;

            inline int found_widget_count() const noexcept { return this->results.total_widget_count; }
            inline const QList<QWidget*>& found_root_widgets() const noexcept { return this->results.widgets; }
            inline const QList<QButtonGroup*>& found_button_groups() const noexcept { return this->results.button_groups; }

            // Control whether the finder aborts, and clears its results, upon finding something that isn't 
            // abandoned. The finder will abort-and-clear by default, as a useful optimization for when the 
            // VM needs to decide what to delete.
            //
            // This option can only safely be used when running on the wrapper teardown thread.
            inline void set_stop_on_referenced(bool b) noexcept {
               this->options.halt_and_clear_upon_non_abandoned = b;
            }
      };
      
      void _setup_lua_vm();
      void _teardown_lua_vm(); // can only safely run on the main thread, since it tears down Qt objects now too

      void _run_queued_functions(bool ui_locked);
      void _script_thread_loop();

      //
      // Returns (true) if the Lua VM should be kept alive even after the script has finished 
      // executing. This would be the case if there are any script-spawned UI windows that are 
      // still open and visible.
      //
      bool _should_keep_running() const noexcept;

      // Members:
      
      std::atomic<bool>   aborted = false; // main thread can set this to kill the script
      std::atomic<bool>   paused  = false; // main thread can set this to pause the script, though it won't take effect instantly. we unpause when running a new script.
      cobb::lockable_bool running = false;
      bool in_teardown = false;
      struct {
         std::mutex pd_mutex; // the (extant) list should not be touched from the non-script thread
         //
         QVector<model_observer_t*> extant;
         QVector<model_observer_t*> pending_deletion;
      } ui_model_observers;
      struct {
         std::mutex pd_mutex; // the (extant) list should not be touched from the non-script thread
         //
         QVector<CanvasWidgetLayerData*> extant;
         QVector<CanvasWidgetLayerData*> pending_deletion;
      } ui_canvas_layer_data;
      QWidget* ui_parent = nullptr;
      
      struct {
         _task_queue s2m; // script-to-main
         struct { // main-to-script
            _task_queue urgent; // urgent messages. these MUST NOT trigger Lua code to execute!
         } m2s;
      } task_queues;
      
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
         std::unordered_map<QObject*, std::unordered_map<std::string, std::unordered_map<std::string, QMetaObject::Connection>>> connections; // connections[widget][event_name][listener] = connection; // can also hold event listeners for non-widgets
         int extant_widget_count = 0; // includes windows
      } widgets;
      struct {
         _task_queue read;  // script-to-main; always blocks
         _task_queue write; // script-to-main; may block
         editor_script::ui_event_queue events; // main-to-script
      } ui_queues;

      //
      // The eventFilter that we use to lock UI interaction can also prevent repaints from occurring 
      // under yet-to-be-determined conditions (I'm not keen on digging through miles of Qt source 
      // code to understand the specifics). If we blindly allow repaint events while the UI is locked, 
      // then we get flickering widgets and other glitchy visual artifacts. Instead, we'll just keep 
      // track of whether we've blocked a repaint, and if so, we'll force one on the main thread as 
      // soon as possible after the UI is unlocked.
      //
      bool repaint_requested_while_ui_locked = false;

      //
      // Whether to override the current UI lock state. Used when we execute functions that Lua has 
      // asked us to run with a particular lock state.
      //
      ui_lock_override_state ui_lock_override = ui_lock_override_state::unchanged;
      
   public:
      static DovahKitScriptVMCore& get() {
         static DovahKitScriptVMCore instance;
         return instance;
      }
      
      lua_State*  lua_vm = nullptr;
      std::thread thread;
      QTimer      main_thread_tick_timer;
      std::atomic<unsigned int> pending_ui_event_count = 0;
      
      inline bool is_aborted() const noexcept { return this->aborted; }
      inline bool is_running() const noexcept { return this->running; }
      inline bool is_paused()  const noexcept { return this->paused; }
      bool teardown_in_progress() const noexcept;

      inline QWidget* get_ui_parent_widget() const noexcept { return this->ui_parent; }

      static void require_script_thread();
      static void require_client_thread(); // actually just requires that it not be the script thread
      static void require_wrapper_teardown_thread();

      // Mark a hierarchy of abandoned widgets for deletion. You should use this after running a 
      // _hierarchy_finder that was configured to halt-and-clear upon finding a non-abandoned 
      // entity. This function doesn't check whether anything in the hierarchy finder is abandoned; 
      // that's the job of the finder itself.
      void mark_abandoned_hierarchy_for_delete(const _hierarchy_finder&);

      void unmark_rescued_hierarchy_for_delete(const _hierarchy_finder&);

      QDialog* try_spawn_script_window() noexcept;
      void set_up_new_scripted_widget(QWidget*);  // Lua functions that create widgets must call this
      void set_up_widget_model(QWidget*);         // Lua functions that create widgets should call this, if the widgets use a model and that model will be relevant for scripting
      void accept_new_orphaned_widget(QWidget*);  // Lua functions that orphan widgets from a window must call this
      void widget_no_longer_orphaned(QWidget*);   // Lua functions that insert widgets into a window must call this
      void widget_no_longer_referenced(QWidget*); // called by wrapper internals when a widget is unreferenced

      QButtonGroup* try_spawn_button_group();
      void button_group_gained_a_member(QButtonGroup*);
      void button_group_lost_a_member(QButtonGroup*);
      void button_group_no_longer_referenced(QButtonGroup*);

      void model_observer_reference_gained(ObservableStandardItemModelObserver*);     // called by userdata-interface internals when a new observer wrapper is created
      void model_observer_no_longer_referenced(ObservableStandardItemModelObserver*); // called by userdata-interface internals when no wrappers for an observer remain

      //
      // Lua APIs that remove and delete items from a ObservableStandardItemModel should call 
      // this function after the removal is complete and control has returned to the script 
      // thread, in order to zombify any extant wrappers for the removed items.
      //
      // When removing a single item, it may be tempting to try and zombify just that one 
      // item's wrapper, but you should be aware that removing a single item can invalidate 
      // multiple observers (and thus require zombifying multiple wrappers): for example, if 
      // you remove a cell from a table with only one column, then you are also removing a 
      // row, and if Lua has accessed that row, it will have a separate observer and wrapper. 
      // There's also just the possibility that I might screw up somewhere, in a way that 
      // would allow a cell to have multiple observers/wrappers, and y'know, we should handle 
      // that case gracefully too!
      //
      void zombify_all_invalid_model_observers();

      void set_up_new_canvas_layer_data(CanvasWidgetLayerData*);   // Lua functions that create CWLDs must call this
      void canvas_layer_data_unreferenced(LuaScriptableCanvasWidgetLayerData*); // called by userdata-interface internals when a CWLD becomes Lua-unreferenced
      void canvas_layer_data_detached(CanvasWidgetLayerData*);     // called by our signal/slot connection when a CWLD becomes Qt-unreferenced

      void queue_lua_function(int stack_pos, bool lock_ui_for_function); // made available for Lua APIs

      int push_to_lua(const QVariant&);
      QVariant variant_from_lua(int stack_pos);
      
   signals:
      void messageLogged(const QString&);
      void scriptStarted();
      void scriptEnded(bool error);
      //
   public:
      void abort();
      void runScript(const QString& code, const QString& name);
      void setPaused(bool);
      void setUIParentWidget(QWidget*); // only allowed when a script is not running
      //
   protected slots:
      void mainThreadLoop();

   protected:
      // Installed on all widgets and windows owned by script. Allows us to block ALL interaction with 
      // scripted UI while a Lua event listener is running.
      virtual bool eventFilter(QObject* object, QEvent* event) override;
};