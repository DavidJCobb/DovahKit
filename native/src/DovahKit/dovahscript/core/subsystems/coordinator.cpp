#include "coordinator.h"
#include <QLabel>
#include <QSortFilterProxyModel>
#include "../../../helpers/lua/dump.h"
#include "../../../helpers/qt/can_have_model.h"
#include "../../../helpers/qt/get_model_of.h"
#include "../../../helpers/qt/set_model_of.h"
#include "../../../helpers/qt/repaint.h"
#include "../../../helpers/set_current_thread_name.h"
#include "../../../ui/generic/CanvasWidget.h"
#include "events.h"
#include "lifetime.h"
#include "resources.h"
#include "userdata.h"
#include "../../dovahscript_host.h"
#include "../../../editor/core.h"

#include "coordinator/client_thread_script_borrow_handle.h"
#include "../verify_threading.h"
#include "../../safe_call.h"

#include "../../lua_libraries/_import_all.h"
#include "../../lua_classes/_import_all.h"
#include "../../wrappers/set_up_all.h"

#include "../../tasks/_base.h"
#include "../../tasks/_ui_base.h"

#include "../../qt/DovahscriptDialog.h"
#include "../../qt/DovahscriptStandardItemModel.h"

#include "../../qt/impl/canvas_context_menu.h"

#include "../../constants/debugging.h"

namespace {
   static constexpr bool debug_script_start_stop = dovahscript::force_enable_debug_logging || false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;

   static constexpr const char* ui_locked_queue_registry_key   = "dovahscript.internal.queued_fucntions.ui_locked";
   static constexpr const char* ui_unlocked_queue_registry_key = "dovahscript.internal.queued_fucntions.ui_unlocked";
}

namespace dovahscript::core::subsystems {
   coordinator::coordinator() {
      this->main_thread_tick_timer.setSingleShot(false);
      this->main_thread_tick_timer.setInterval(0);
      QObject::connect(&this->main_thread_tick_timer, &QTimer::timeout, this, &coordinator::_main_thread_loop);
      QObject::connect(this, &coordinator::_internal_scriptDone, this, [this]() {
         if constexpr (debug_script_start_stop) {
            qDebug("dovahscript::core::subsystems::coordinator received its own scriptEnded signal...");
         }
         this->main_thread_tick_timer.stop();
         this->_teardown_lua_state();
         //
         emit host::get().scriptEnded();
      });
      //
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool will_be_flagged) {
         if (!will_be_flagged)
            return;
         auto& list = this->expected_deletions;
         auto  it   = std::find(list.begin(), list.end(), stub);
         assert(it != list.end() && "Form stubs should never be deleted while a script is running, except as the result of a delete_form task!");
         list.erase(it);
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &coordinator::abort);
   }
   coordinator::~coordinator() {
      this->abort();
      if (this->worker_thread.joinable()) // even if it's finished running, we need to join it or std::thread::operator= below will break
         this->worker_thread.join();
   }

   void coordinator::_main_thread_loop() {
      if (!this->running)
         return;
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
      if (this->aborted) {
         //
         // Do not run cross-thread tasks if an abort has been called. Partly that's because 
         // we don't want the script to be able to do very much once the user has decided to 
         // abort it. Partly it's to avoid a nasty race condition within the task system. 
         // Refer to comments on the "send task" functions.
         //
         this->task_queues.s2m.clear();
         this->task_queues.ui.write.clear();
         this->task_queues.blocking.discard();
         return;
      } else {
         this->task_queues.s2m.process();
         this->task_queues.ui.write.process();
         this->task_queues.blocking.process();
      }
   }

   void coordinator::_run_eval_script(const QString& code) {
      auto* L    = this->lua_state;
      auto  top  = lua_gettop(L);
      auto& host = host::get();
      //
      auto buffer = code.toUtf8();
      auto result = luaL_loadbufferx(L, buffer.data(), buffer.size(), "<eval>", "t"); // equivalent to (lua_load) with a built-in lua_Reader
      //
      switch (result) {
         case LUA_OK:
            if constexpr (debug_script_start_stop) {
               qDebug("Parsed eval script.");
            }
            break;
         case LUA_ERRMEM:
         case LUA_ERRSYNTAX:
         default:
            if constexpr (debug_script_start_stop) {
               qDebug("Failed to parse eval script.");
            }
            auto message = QString::fromUtf8(lua_tostring(L, -1));
            emit host.messageLogged(message);
            emit host.evalComplete();
            return;
      }
      if (lua_gettop(L) == top)
         return;
      safe_call(L, 0, LUA_MULTRET);
      emit host.evalComplete();
      //
      int end   = lua_gettop(L);
      int count = end - top;
      if (count > 0) {
         emit host.messageLogged(QString("Eval script ran to completion and returned %1 value(s):").arg(count));
         for (int i = top + 1; i <= end; ++i) {
            auto s = cobb::lua::var_to_string(L, i);
            emit host.messageLogged(QString::fromStdString(s));
         }
         lua_settop(L, top);
      } else {
         emit host.messageLogged("Eval script ran to completion.");
      }
   }
   void coordinator::_script_thread_loop() {
      assert(this->worker_thread_state == thread_wait_state::running); // After running one session and when running a new one, this should be reset before the worker thread is created.
      this->script_thread = thread_type::worker;
      cobb::set_current_thread_name(L"Dovahscript");
      //
      // Run all outstanding script files:
      //
      for (auto& script : this->scripts_to_run.files) {
         auto buffer = script.contents.toUtf8();
         auto result = luaL_loadbufferx(this->lua_state, buffer.data(), buffer.size(), script.filename.toUtf8().data(), "t"); // equivalent to (lua_load) with a built-in lua_Reader
         script.contents.clear();
         //
         switch (result) {
            case LUA_OK:
               if constexpr (debug_script_start_stop) {
                  qDebug("Successfully parsed script: %s", script.filename.toUtf8());
               }
               safe_call(this->lua_state, 0, 0);
               break;
            case LUA_ERRMEM:
            case LUA_ERRSYNTAX:
            default:
               if constexpr (debug_script_start_stop) {
                  qDebug("Failed to parse script: %s", script.filename.toUtf8());
               }
               auto message = QString::fromUtf8(lua_tostring(this->lua_state, -1));
               emit host::get().messageLogged(message);
               break;
         }
      }
      this->scripts_to_run.files.clear();
      if constexpr (debug_script_start_stop) {
         qDebug("Finished handling all requested script files. Switching to script thread idle loop.");
      }
      //
      // All script files are executed. Dip into the idle loop now:
      //
      bool ran_more_code;
      do {
         this->_do_worker_thread_wait();
         //
         ran_more_code = false;
         this->task_queues.s2m.wait_until_empty();      // these can be non-blocking + fire-and-forget
         this->task_queues.ui.write.wait_until_empty(); // these can be non-blocking + fire-and-forget
         ran_more_code |= (this->_run_queued_functions(false) > 0);
         ran_more_code |= (events::get().process_pending_events() > 0);
         ran_more_code |= (this->_run_queued_functions(true) > 0);
         {
            QString eval;
            {
               auto& state = this->eval_script_state;
               auto  guard = std::lock_guard(state.lock);
               std::swap(eval, state.code);
            }
            if (!eval.isEmpty())
               this->_run_eval_script(eval);
         }
         lifetime::get().worker_thread_handler();
      } while (ran_more_code || this->_should_keep_running());
      //
      if constexpr (debug_script_start_stop) {
         qDebug("Script execution finished on the worker thread.");
      }
      //this->main_thread_tick_timer.stop(); // can only stop timers from their owning threads
      this->running = false;
      this->worker_thread_state = coordinator::thread_wait_state::finished;
      emit this->_internal_scriptDone(); // a main-thread handler will catch this and tear down the VM
   }

   bool coordinator::_should_keep_running() const noexcept {
      if (this->aborted)
         return false;
      //
      // If the script has any script-spawned UI windows open and visible, then this function 
      // should return (true). If we want to be more sophisticated, then we can double-check 
      // that the windows or any controls in them have any event listeners registered.
      //
      // The basic thing we're checking for is, "We're not running script code *right now*, 
      // but can we *end up* running them as a result of any extant event listeners?"
      //
      if (lifetime::get().any_windows_visible_or_task_referenced())
         return true;
      return false;
   }

   void coordinator::_do_worker_thread_wait() {
      this->outstanding_client_thread_script_borrow_requests.wait_until_zero(
         [this]() { // Pre-wait
            assert(this->script_thread == thread_type::worker);
            this->script_thread       = thread_type::client;
            this->worker_thread_state = coordinator::thread_wait_state::waiting;
         },
         [this]() { // Post-wait
            this->worker_thread_state = coordinator::thread_wait_state::running;
            this->script_thread       = thread_type::worker;
         }
      );
   }

   /*static*/ void coordinator::_lua_debug_hook(lua_State* L, lua_Debug* ar) {
      auto& s = coordinator::get();
      if (s.script_thread == thread_type::worker) {
         lifetime::get().worker_thread_handler();
         while (s.is_paused())
            if (s.is_aborted())
               break;
         s._do_worker_thread_wait();
      }
      if (s.is_aborted()) {
         lua_getfield(L, LUA_REGISTRYINDEX, coordinator::abort_sentinel_userdata);
         lua_error(L);
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
      assert(this->expected_modifications.empty());
      //
      this->lua_state = luaL_newstate();
      auto* L = this->lua_state;
      lua_sethook (L, &_lua_debug_hook, LUA_MASKCOUNT, 8);
      lua_setwarnf(L, &_lua_warning_function, nullptr);
      //
      // Create "abort" sentinel error:
      //
      lua_newuserdata(L, 1);
      lua_setfield(L, LUA_REGISTRYINDEX, abort_sentinel_userdata);
      //
      // Import libraries and set up subsystems:
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
      events::get().initialize(L);
      lifetime::get().on_script_setup();
      resources::get().on_script_setup();
      userdata::get().initialize(L);
      //
      set_up_all_native_wrappers(L);
      //
      this->ui_lock_override = ui_lock_override_state::unchanged;
   }
   void coordinator::_teardown_lua_state() { // can only safely run on the client thread, since it tears down Qt objects now too
      require_client_thread();
      assert(this->worker_thread_state == coordinator::thread_wait_state::finished);
      this->script_thread = thread_type::client;
      //
      this->in_teardown = true;
      if constexpr (debug_script_start_stop) {
         qDebug("Tearing down the script VM...");
      }
      this->main_thread_tick_timer.stop();
      if (auto* L = this->lua_state) {
         lua_close(L);
         this->lua_state = nullptr;
      }
      {
         auto guard = std::lock_guard(this->eval_script_state.lock);
         this->eval_script_state.code.clear();
      }
      //
      this->task_queues.s2m.clear();
      this->task_queues.ui.write.clear();
      this->task_queues.blocking.discard();
      {
         for (auto* stub : this->expected_modifications)
            emit DovahKitCore::get().formModified(stub);
         this->expected_modifications.clear();
      }
      //
      events::get().on_script_teardown();
      lifetime::get().on_script_teardown();
      resources::get().on_script_teardown();
      //
      if (this->worker_thread.joinable())
         this->worker_thread.join();
      this->worker_thread_state = thread_wait_state::running;
      //
      this->in_teardown = false;
   }

   void coordinator::_lua_warning_function(void* ud, const char* msg, int tocont) {
      static QString text; // we could use (ud) to hold this, but since the VM is itself a singleton, no point in trying to allow multiple warning handlers to exist simultaneously
      static bool    fragment = false;
      //
      if (!fragment) {
         text = "[Warning] ";
      }
      fragment = tocont;
      text += msg;
      if (!tocont) {
         auto* L = coordinator::get().lua_state;
         luaL_traceback(L, L, text.toUtf8(), 0);
         if (lua_isstring(L, -1))
            text = lua_tostring(L, -1);
         lua_pop(L, 1);
         //
         host::get().messageLogged(text);
         text.clear();
      }
   }

   QWidget* coordinator::get_ui_parent() const noexcept {
      return this->ui_parent;
   }
   void coordinator::set_ui_parent(QWidget* p) noexcept {
      assert(!this->is_running());
      assert(!this->in_teardown);
      //
      this->ui_parent = p;
   }

   bool coordinator::teardown_in_progress() const noexcept {
      return this->in_teardown;
   }

   // For important implementation information on this and the other "send task" 
   // functions, refer to the <cross-thread tasks and script aborts.txt> file 
   // in the documentation folder.
   void coordinator::_clear_all_task_queues() {
      this->task_queues.s2m.clear();
      this->task_queues.ui.write.clear();
   }
   void coordinator::_send_blocking_task(task_queue::task_t& task) {
      thread_type prior;
      bool needs_lua = task.needs_lua_ownership();
      if (needs_lua) {
         prior = this->script_thread;
         this->script_thread = thread_type::client;
         task.run_lua_before(this->lua_state);
      }
      this->task_queues.blocking.send(task);
      task.seen.wait(false);
      if (needs_lua)
         this->script_thread = prior;
   }
   void coordinator::send_script_task(task_queue::task_t& task) {
      require_script_thread();
      if (this->aborted)
         return;
      if (task.is_blocking()) {
         this->_send_blocking_task(task);
         return;
      }
      this->task_queues.s2m.push_back(&task);
   }
   void coordinator::send_ui_read_task(tasks::_ui_read_base& task) {
      require_script_thread();
      if (this->aborted)
         return;
      this->task_queues.ui.write.wait_until_empty();
      assert(task.is_blocking());
      this->_send_blocking_task(task);
   }
   void coordinator::send_ui_write_task(tasks::_ui_write_base& task) {
      require_script_thread();
      if (this->aborted)
         return;
      if (task.is_blocking()) {
         this->_send_blocking_task(task);
         return;
      }
      this->task_queues.ui.write.push_back(&task);
   }


   void coordinator::abort() {
      require_client_thread();
      //
      auto guard = std::lock_guard(this->start_stop_lock);
      if (this->running)
         this->aborted = true;
   }
   void coordinator::execute_scripts(const script_set& scripts) {
      auto guard = std::lock_guard(this->start_stop_lock);
      if (this->running) {
         if constexpr (debug_script_start_stop) {
            qDebug("Failed to start script: another script is already running.");
         }
         return;
      }
      this->scripts_to_run = scripts;
      if (this->worker_thread.joinable()) // even if it's finished running, we need to join it or std::thread::operator= below will break
         this->worker_thread.join();
      this->aborted = false;
      this->running = true;
      this->paused  = false;
      if constexpr (debug_script_start_stop) {
         qDebug("Starting a new script...");
      }
      this->main_thread_tick_timer.start();
      emit host::get().scriptStarted();
      //this->_teardown_lua_vm();
      this->_setup_lua_state();
      //
      this->worker_thread = std::thread([this]() { this->_script_thread_loop(); });
   }
   bool coordinator::eval_script(const QString& code) {
      if (code.isEmpty())
         return false;
      auto& state = this->eval_script_state;
      auto  guard = std::lock_guard(state.lock);
      if (!state.code.isEmpty())
         return false;
      state.code = code;
      return true;
   }
   void coordinator::set_pause_state(bool b) {
      this->paused = b;
      if constexpr (debug_script_start_stop) {
         qDebug("Setting script pause state to: %d", b);
      }
   }

   void coordinator::create_model_for_widget(QWidget& widget) {
      require_client_thread();
      //
      auto* model = new qt_model_type(&widget);
      auto* proxy = new QSortFilterProxyModel(model);
      proxy->setSourceModel(model);
      proxy->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
      cobb::qt::set_model_of(&widget, proxy);
      model->associateWithWidget(&widget);
   }
   void coordinator::force_ui_repaint() {
      require_client_thread();
      //
      auto list = lifetime::get().get_script_windows();
      for (auto* window : list)
         cobb::qt::update_hierarchy(window);
   }

   void coordinator::set_up_widget(QWidget& widget) {
      require_client_thread();
      //
      widget.installEventFilter(this);
      if (auto* label = qobject_cast<QLabel*>(&widget)) {
         QObject::connect(label, &QLabel::linkActivated, [label](const QString& url) {
            emit host::get().userClickedLink(url, label->window());
         });
      } else if (auto* canvas = qobject_cast<CanvasWidget*>(&widget)) {
         dovahscript::impl::set_up_canvas_context_menu(canvas);
      }
      if (cobb::qt::can_have_model(&widget))
         this->create_model_for_widget(widget);
   }

   void coordinator::expect_deletion_of(passkey_to<tasks::s2m::delete_form>, const std::vector<dovah::form_stub*>& append) {
      require_client_thread();
      require_script_thread();
      //
      auto& list = this->expected_deletions;
      list.insert(list.begin(), append.begin(), append.end());
   }
   void coordinator::on_deletion_completion_expected(passkey_to<tasks::s2m::delete_form>) {
      require_client_thread();
      require_script_thread();
      //
      auto& list = this->expected_deletions;
      assert(list.empty() && "A delete_form task didn't delete all of the forms it expected to delete!");
   }


   void coordinator::expect_modification_of(passkey_to<tasks::s2m::internal_signal_form_edit>, dovah::form_stub& stub) {
      require_client_thread();
      //
      auto& list = this->expected_modifications;
      list.push_back(&stub);
   }
   void coordinator::on_modification_complete(passkey_to<tasks::s2m::internal_signal_form_edit>, dovah::form_stub& stub) {
      require_client_thread();
      //
      auto& list = this->expected_modifications;
      auto  it   = std::find(list.begin(), list.end(), &stub);
      assert(it != list.end() && "An internal_signal_form_edit task failed to tell us to expect the modification of a form!");
      list.erase(it);
   }
}