#pragma once
#include <QTimer>
#include "../../../lua.h"
#include "../../../helpers/passkey.h"
#include "../../../helpers/singleton.h"
#include "../../../dovah/core.h"
#include "../blocking_task_slot.h"
#include "../task_queue.h"
#include "../../script_set.h"

class DovahscriptStandardItemModel;
namespace dovah {
   class form_stub;
}
namespace dovahscript {
   namespace core::subsystems {
      class coordinator;
   }
   namespace tasks {
      class _ui_read_base;
      class _ui_write_base;
      namespace s2m {
         class delete_form;
         class internal_signal_form_edit;
      }
   }
}

namespace dovahscript::core {
   enum class thread_type {
      client,
      worker,
   };

   class client_thread_script_borrow_handle;
}

namespace dovahscript::core::subsystems {
   class coordinator : public QObject, cobb::singleton {
      Q_OBJECT;
      friend class client_thread_script_borrow_handle;
      public:
         inline static coordinator& get() {
            static coordinator instance;
            return instance;
         }

         template<typename T> using passkey_to = cobb::passkey<coordinator, T>;

         using qt_model_type = DovahscriptStandardItemModel;
         enum class ui_lock_override_state {
            unchanged,
            locked,
            unlocked,
         };

         inline static constexpr const char* abort_sentinel_userdata = "dovahscript.internal.abort_error";

      protected:
         enum class thread_wait_state {
            running,
            waiting,
            finished,
         };

      signals:
         void _internal_scriptDone();

      protected:
         coordinator();
         ~coordinator();

         std::atomic<bool> aborted = false; // main thread can set this to kill the script
         std::atomic<bool> paused  = false; // main thread can set this to pause the script, though it won't take effect instantly. we unpause when running a new script.
         std::atomic<bool> running = false;
         bool in_teardown = false;
         std::mutex start_stop_lock; // used for any function that the outside world would use to start or abort a script

         std::atomic<thread_wait_state> worker_thread_state = thread_wait_state::running;
         std::atomic<int> outstanding_client_thread_script_borrow_requests = 0;

         script_set scripts_to_run;
         struct {
            QString    code; // cleared once the code is run
            std::mutex lock;
         } eval_script_state;
         struct {
            task_queue s2m;
            struct {
               task_queue write;
            } ui;
            blocking_task_slot blocking;
         } task_queues;

         // Access from the client thread only:
         std::vector<dovah::form_stub*> expected_deletions;     // Detect when form stubs are deleted out from under the script engine, so we can assert that that never happens.
         std::vector<dovah::form_stub*> expected_modifications; // Handle the case of the script being aborted while a form modification is in progress, so we don't leave the form in limbo.

         // Parent widget outside of the script engine, which scripted windows should be children of.
         QWidget* ui_parent = nullptr;

         //
         // The eventFilter that we use to lock UI interaction can also prevent repaints from occurring 
         // under yet-to-be-determined conditions (I'm not keen on digging through miles of Qt source 
         // code to understand the specifics). If we blindly allow repaint events while the UI is locked, 
         // then we get flickering widgets and other glitchy visual artifacts. Instead, we'll just keep 
         // track of whether we've blocked a repaint, and if so, we'll force one on the main thread as 
         // soon as possible after the UI is unlocked.
         // 
         // This bool is only safely accessible from the client thread.
         //
         bool repaint_requested_while_ui_locked = false;

         //
         // Whether to override the current UI lock state. Used when we execute functions that Lua has 
         // asked us to run with a particular lock state.
         //
         ui_lock_override_state ui_lock_override = ui_lock_override_state::unchanged;
         
         void _setup_lua_state();
         void _teardown_lua_state(); // can only safely run on the client thread, since it tears down Qt objects now too

         int _run_queued_functions(bool ui_locked); // returns the number of functions executed
         void _run_eval_script(const QString&);
         void _script_thread_loop();

         //
         // Returns (true) if the Lua VM should be kept alive even after the script has finished 
         // executing. This would be the case if there are any script-spawned UI windows that are 
         // still open and visible.
         //
         bool _should_keep_running() const noexcept;

         void _do_worker_thread_wait();

         static void _lua_debug_hook(lua_State* L, lua_Debug* ar);

         static void _lua_warning_function(void* ud, const char* msg, int tocont);

         void _clear_all_task_queues();
         void _send_blocking_task(task_queue::task_t&);

      public:
         lua_State*      lua_state = nullptr;
         std::thread     worker_thread;
         std::thread::id client_thread_id;
         thread_type     script_thread = thread_type::client;
         QTimer          main_thread_tick_timer;

         QWidget* get_ui_parent() const noexcept;
         void set_ui_parent(QWidget*) noexcept;

         inline bool is_aborted() const noexcept { return this->aborted; }
         inline bool is_running() const noexcept { return this->running; }
         inline bool is_paused()  const noexcept { return this->paused; }
         bool teardown_in_progress() const noexcept;

         // Script thread functions:

         void send_script_task(task_queue::task_t&);
         void send_ui_read_task(tasks::_ui_read_base&);
         void send_ui_write_task(tasks::_ui_write_base&);

         // Client thread functions:

         void abort();
         void execute_scripts(const script_set&);
         bool eval_script(const QString&); // returns true if the eval script could be queued. asynch, so it will likely return before the eval script runs.
         void set_pause_state(bool);

         void create_model_for_widget(QWidget&);
         void force_ui_repaint();

         // Sets up non-lifetime-related behaviors and defaults on some widgets. Should be called for 
         // every newly-created scripted widget.
         void set_up_widget(QWidget&);

         void expect_deletion_of(passkey_to<tasks::s2m::delete_form>, const std::vector<dovah::form_stub*>&);
         void on_deletion_completion_expected(passkey_to<tasks::s2m::delete_form>);

         void expect_modification_of(passkey_to<tasks::s2m::internal_signal_form_edit>, dovah::form_stub&);
         void on_modification_complete(passkey_to<tasks::s2m::internal_signal_form_edit>, dovah::form_stub&);
         
      protected slots:
         void _main_thread_loop();
   };
}