#pragma once
#include <type_traits>
#include "../core/subsystems/coordinator.h"
#include "../tasks/s2m/ui_read_lambda.h"
#include "../tasks/s2m/ui_write_lambda.h"

namespace dovahscript::api_helpers {
   template<class W, class Wx, typename R> requires (std::is_base_of_v<Wx, W>) R get_widget_property(const W* widget_bare, R (Wx::* func)() const) {
      R result;
      {
         task_reference widget = widget_bare;
         //
         auto* task = new tasks::s2m::ui_read_lambda();
         task->handler = [widget, func, &result]() {
            result = (widget->*func)();
         };
         core::subsystems::coordinator::get().send_ui_read_task(*task);
         delete task;
      }
      return result;
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Use this for when 
   // the argument should be passed by reference.
   //
   template<class W, class Wx, class T> requires (std::is_base_of_v<Wx, W>) void set_widget_property(W* widget_bare, void (Wx::* func)(const T&), const T& value) {
      task_reference widget = widget_bare;
      //
      auto* task = new tasks::s2m::ui_write_lambda(false);
      task->handler = [widget, value, func]() {
         (widget->*func)(value);
      };
      core::subsystems::coordinator::get().send_ui_write_task(*task);
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Separate template 
   // types are used for the QWidget function argument (T) and for the value (U) so that you can 
   // pass in inexact matches, e.g. passing a lua_Integer value to a function that takes int.
   //
   template<class W, class Wx, class T, class U> requires (std::is_base_of_v<Wx, W> && std::is_convertible_v<T, U> && !std::is_reference_v<T>) void set_widget_property(W* widget_bare, void (Wx::* func)(T), U value) {
      task_reference widget = widget_bare;
      //
      auto* task = new tasks::s2m::ui_write_lambda(false);
      task->handler = [widget, value, func]() {
         (widget->*func)(value);
      };
      core::subsystems::coordinator::get().send_ui_write_task(*task);
   }

   
   template<class W, class Wx, class T> requires (std::is_base_of_v<Wx, W>)
   void set_widget_property_and_block_signals(W* widget, void (Wx::* func)(const T&), const T& value) {
      task_reference widget = widget_bare;
      //
      auto* task = new tasks::s2m::ui_write_lambda(false);
      task->handler = [widget, value, func]() {
         const auto blocker = QSignalBlocker(widget);
         (widget->*func)(value);
      };
      core::subsystems::coordinator::get().send_ui_write_task(*task);
   }
   template<class W, class Wx, class T, class U> requires (std::is_base_of_v<Wx, W>&& std::is_convertible_v<T, U> && !std::is_reference_v<T>)
   void set_widget_property_and_block_signals(W* widget_bare, void (Wx::* func)(T), U value) {
      task_reference widget = widget_bare;
      //
      auto* task = new tasks::s2m::ui_write_lambda(false);
      task->handler = [widget, value, func]() {
         const auto blocker = QSignalBlocker(widget);
         (widget->*func)(value);
      };
      core::subsystems::coordinator::get().send_ui_write_task(*task);
   }
}