#pragma once
#include <QTimer>
#include "../../../lua.h"
#include "../../../helpers/lockable_bool.h"
#include "../../../helpers/singleton.h"
#include "../task_queue.h"

namespace dovahscript::core {
   enum class thread_type {
      client,
      worker,
   };
}

namespace dovahscript::core::subsystems {
   class coordinator : public QObject, cobb::singleton {
      Q_OBJECT;
      public:
         inline static coordinator& get() {
            static coordinator instance;
            return instance;
         }

      protected:
         std::atomic<bool>   aborted = false; // main thread can set this to kill the script
         std::atomic<bool>   paused  = false; // main thread can set this to pause the script, though it won't take effect instantly. we unpause when running a new script.
         cobb::lockable_bool running = false;
         bool in_teardown = false;

         struct {
            task_queue s2m;
            struct {
               task_queue read;
               task_queue write;
            } ui;
         } task_queues;

         // Parent widget outside of the script engine, which scripted windows should be children of.
         QWidget* ui_parent = nullptr;
         
         void _setup_lua_vm();
         void _teardown_lua_vm(); // can only safely run on the main thread, since it tears down Qt objects now too

         int _run_queued_functions(bool ui_locked); // returns the number of functions executed
         void _script_thread_loop();

         //
         // Returns (true) if the Lua VM should be kept alive even after the script has finished 
         // executing. This would be the case if there are any script-spawned UI windows that are 
         // still open and visible.
         //
         bool _should_keep_running() const noexcept;

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
         
      protected slots:
         void mainThreadLoop();
   };
}