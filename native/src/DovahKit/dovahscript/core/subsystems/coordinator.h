#pragma once
#include <QTimer>
#include "../../../lua.h"
#include "../../../helpers/lockable_bool.h"
#include "../../../helpers/singleton.h"
#include "../task_queue.h"

namespace dovahscript::core::subsystems {
   class coordinator;
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

      protected:
         enum class thread_wait_state {
            running,
            waiting,
         };

      protected:
         std::atomic<bool>   aborted = false; // main thread can set this to kill the script
         std::atomic<bool>   paused  = false; // main thread can set this to pause the script, though it won't take effect instantly. we unpause when running a new script.
         cobb::lockable_bool running = false;
         bool in_teardown = false;

         std::atomic<thread_wait_state> worker_thread_state;
         std::atomic<int> outstanding_client_thread_script_borrow_requests = 0;

         struct {
            task_queue s2m;
            struct {
               task_queue read;
               task_queue write;
            } ui;
         } task_queues;

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
         
         void _setup_lua_vm();
         void _teardown_lua_vm(); // can only safely run on the client thread, since it tears down Qt objects now too

         int _run_queued_functions(bool ui_locked); // returns the number of functions executed
         void _script_thread_loop();

         //
         // Returns (true) if the Lua VM should be kept alive even after the script has finished 
         // executing. This would be the case if there are any script-spawned UI windows that are 
         // still open and visible.
         //
         bool _should_keep_running() const noexcept;

         static void _lua_debug_hook(lua_State* L, lua_Debug* ar);

      public:
         lua_State*      lua_vm = nullptr;
         std::thread     worker_thread;
         std::thread::id client_thread_id;
         thread_type     script_thread = thread_type::client;
         QTimer          main_thread_tick_timer;

         inline bool is_aborted() const noexcept { return this->aborted; }
         inline bool is_running() const noexcept { return this->running; }
         inline bool is_paused()  const noexcept { return this->paused; }
         bool teardown_in_progress() const noexcept;

         // The client thread can call this function in order to ask the worker thread to pause and wait, 
         // so that the client thread can perform some task using the Lua state. The client thread should 
         // check the "is valid" and "is ready" values on the returned handle, and should execute whatever 
         // code it wanted to execute once the handle tests as "ready."
         //
         // Internally, we have a counter indicating how many extant handles there are -- how many reasons 
         // the client thread wants to run code on the script thread. The handle destructors manage this 
         // counter, and when it drops to zero, the worker thread will resume being the script thread and 
         // will be allowed to proceed.
         client_thread_script_borrow_handle borrow_script_thread_status();
         
      protected slots:
         void _main_thread_loop();
   };
}