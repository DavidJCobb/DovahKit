#include "coordinator.h"
#include <QSortFilterProxyModel>
#include "../../../helpers/qt/get_model_of.h"
#include "../../../helpers/qt/set_model_of.h"
#include "events.h"
#include "lifetime.h"
#include "resources.h"
#include "userdata.h"

#include "coordinator/client_thread_script_borrow_handle.h"
#include "../verify_threading.h"
#include "../../safe_call.h"

#include "../../lua_libraries/_import_all.h"
#include "../../lua_classes/_import_all.h"

#include "../../tasks/_base.h"
#include "../../tasks/_ui_base.h"

#include "../../qt/DovahscriptStandardItemModel.h"

namespace {
   static constexpr bool debug_script_start_stop = false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;
}

namespace dovahscript::core::subsystems {
   void coordinator::_main_thread_loop() {
      auto& lifetime_s = lifetime::get();
      //
      resources::get().main_thread_handler();
      if (this->repaint_requested_while_ui_locked) {
         auto pending = events::get().get_pending_event_count();
         if (!pending) {
            this->repaint_requested_while_ui_locked = false;
            for (auto* w : lifetime_s.get_script_windows())
               w->update();
         }
      }
      lifetime_s.main_thread_handler();
      this->task_queues.s2m.process();
      this->task_queues.ui.read.process();
      this->task_queues.ui.write.process();
   }

   void coordinator::_script_thread_loop() {
      assert(this->worker_thread_state == coordinator::thread_wait_state::running); // After running one session and when running a new one, this should be reset before the worker thread is created.
      //
      static_assert(false, "Run all outstanding script files here.");
      if constexpr (debug_script_start_stop) {
         qDebug("Finished executing all requested script files. Switching to script thread idle loop.");
      }
      //
      bool had_any_tasks;
      do {
         this->task_queues.s2m.wait_until_empty();      // these can be non-blocking + fire-and-forget
         this->task_queues.ui.write.wait_until_empty(); // these can be non-blocking + fire-and-forget
         this->task_queues.m2s.urgent.process();
         had_any_tasks |= (this->_run_queued_functions(false) > 0);
         had_any_tasks |= (events::get().process_pending_events() > 0);
         had_any_tasks |= (this->_run_queued_functions(true) > 0);
      } while (had_any_tasks || this->_should_keep_running());
      //
      if constexpr (debug_script_start_stop) {
         qDebug("Script execution finished on the worker thread.");
      }
      this->main_thread_tick_timer.stop();
      this->running = false;
      this->worker_thread_state = coordinator::thread_wait_state::waiting;
      emit this->scriptEnded(false); // a main-thread handler will catch this and tear down the VM
   }

   bool coordinator::_should_keep_running() const noexcept;

   /*static*/ void coordinator::_lua_debug_hook(lua_State* L, lua_Debug* ar) {
      auto& s = coordinator::get();
      while (s.is_paused())
         if (s.is_aborted())
            break;
      {
         auto& counter = s.outstanding_client_thread_script_borrow_requests;
         if (counter) {
            s.worker_thread_state = coordinator::thread_wait_state::waiting;
            while (counter) {}
            s.worker_thread_state = coordinator::thread_wait_state::running;
         }
      }
      if (s.is_aborted()) {
         static_assert(false, "TODO: Use a unique, pre-created userdata object to signal the error, instead of a string.");
         luaL_error(L, "Script terminated at the user's request.");
      }
   }

   int coordinator::_run_queued_functions(bool ui_locked) {
      require_worker_thread();
      require_script_thread();
      //
      auto start      = lua_gettop(this->lua_state);
      auto index_list = start + 1;
      auto index_nk   = start + 2;
      auto index_nv   = start + 3;
      //
      this->ui_lock_override = ui_locked ? ui_lock_override_state::locked : ui_lock_override_state::unlocked;
      //
      auto* key = ui_locked ? ui_locked_queue_registry_key : ui_unlocked_queue_registry_key;
      auto* L   = this->lua_state;
      int   count_executed = 0;
      if (lua_getfield(L, LUA_REGISTRYINDEX, key) == LUA_TTABLE) {
         int count = lua_rawlen(L, -1);
         if (count) {
            //
            // Clear the list out of the registry (replace it with a blank table), leaving the original list 
            // on the stack for us to use here.
            //
            lua_createtable(L, 0, 0);
            lua_setfield(L, LUA_REGISTRYINDEX, key);
            //
            // Execute each individual function in the list.
            //
            for (int i = 0; i < count; ++i) {
               lua_geti(L, -1, i + 1); // get the function
               safe_call(this->lua_state, 0, 0); // this will pop the function
            }
         }
         count_executed += count;
      }
      lua_settop(this->lua_state, start);
      //
      this->ui_lock_override = ui_lock_override_state::unchanged;
      return count_executed;
   }

   void coordinator::_setup_lua_state() {
      assert(this->lua_state == nullptr);
      assert(!this->in_teardown);
      //
      this->lua_state = luaL_newstate();
      auto* L = this->lua_state;
      lua_sethook (L, &_lua_debug_hook, LUA_MASKCOUNT, 8);
      lua_setwarnf(L, &_lua_warning_function, nullptr);
      //
      lua_libraries::import_all(L);
      lua_classes::import_all(L);
      //
      #pragma region Queued functions
         lua_newtable(L);
         lua_setfield(L, LUA_REGISTRYINDEX, ui_locked_queue_registry_key);
         lua_newtable(L);
         lua_setfield(L, LUA_REGISTRYINDEX, ui_unlocked_queue_registry_key);
      #pragma endregion
      static_assert(false, "TODO: Initialize other subsystems for this Lua state? (They should also take this as a chance to assert whatever they should assert on setup.)");
      userdata::get().initialize(L);
      //
      static_assert(false, "TODO: Build all metatables for form classes.");
      static_assert(false, "TODO: Build all metatables and singletons for UI classes.");
      this->ui_lock_override = ui_lock_override_state::unchanged;
   }
   void coordinator::_teardown_lua_state() {// can only safely run on the client thread, since it tears down Qt objects now too
      require_client_thread();
      assert(this->worker_thread_state == coordinator::thread_wait_state::waiting);
      this->script_thread = thread_type::client;
      //
      auto guard = std::lock_guard(this->running);
      this->in_teardown = true;
      if constexpr (debug_script_start_stop) {
         qDebug("Tearing down the script VM...");
      }
      if (auto* L = this->lua_state) {
         lua_close(L);
         this->lua_state = nullptr;
      }
      //
      events::get().on_script_teardown();
      lifetime::get().on_script_teardown();
      resources::get().on_script_teardown();
      //
      this->in_teardown = false;
   }


   void coordinator::send_script_task(task_queue::task_t& task) {
      require_script_thread();
      //
      this->task_queues.s2m.push_back(&task);
   }
   void coordinator::send_ui_read_task(tasks::_ui_read_base& task) {
      require_script_thread();
      //
      this->task_queues.ui.write.wait_until_empty();
      this->task_queues.ui.read.push_back(&task);
      while (!task.seen)
         if (this->is_aborted())
            break;
      if (!this->is_aborted()) {
         this->task_queues.m2s.urgent.process();
      }
   }
   void coordinator::send_ui_write_task(tasks::_ui_write_base& task) {
      require_script_thread();
      //
      bool blocking = task.is_blocking(); // grab this before adding it to the list, to avoid race conditions (e.g. the main thread executing and deleting a non-blocking task before we get a chance to check)
      this->task_queues.ui.read.wait_until_empty();
      this->task_queues.ui.write.push_back(&task);
      if (blocking) {
         while (!task.seen)
            if (this->is_aborted())
               break;
      }
      if (!this->is_aborted()) {
         this->task_queues.m2s.urgent.process();
      }
   }

   void coordinator::create_model_for_widget(QWidget& widget) {
      require_client_thread();
      //
      auto* model = new qt_model_type(widget);
      auto* proxy = new QSortFilterProxyModel(model);
      proxy->setSourceModel(model);
      proxy->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
      cobb::qt::set_model_of(&widget, proxy);
      model->associateWithWidget(&widget);
   }
}