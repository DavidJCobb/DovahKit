#pragma once
#include <type_traits>
#include "../../../systems/messaging.h"
#include "../../../cross_thread_tasks/s2m/lambda.h"

namespace editor_script::helpers {
   template<class W, class Wx, typename R> requires (std::is_base_of_v<Wx, W>) R get_widget_property(const W* widget, R (Wx::* func)() const) {
      R result;
      {
         auto* task = new tasks::s2m::ui_read_lambda();
         task->handler = [widget, func, &result]() { result = (widget->*func)(); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
      }
      return result;
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Use this for when 
   // the argument should be passed by reference.
   //
   template<class W, class Wx, class T> requires (std::is_base_of_v<Wx, W>) void set_widget_property(W* widget, void (Wx::* func)(const T&), const T& value) {
      auto* task = new tasks::s2m::lambda(false);
      task->handler = [widget, value, func]() {
         (widget->*func)(value);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Separate template 
   // types are used for the QWidget function argument (T) and for the value (U) so that you can 
   // pass in inexact matches, e.g. passing a lua_Integer value to a function that takes int.
   //
   template<class W, class Wx, class T, class U> requires (std::is_base_of_v<Wx, W> && std::is_convertible_v<T, U> && !std::is_reference_v<T>) void set_widget_property(W* widget, void (Wx::* func)(T), U value) {
      auto* task = new tasks::s2m::lambda(false);
      task->handler = [widget, value, func]() { (widget->*func)(value); };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
   }

   
   template<class W, class Wx, class T> requires (std::is_base_of_v<Wx, W>)
   void set_widget_property_and_block_signals(W* widget, void (Wx::* func)(const T&), const T& value) {
      auto* task = new tasks::s2m::lambda(false);
      task->handler = [widget, value, func]() {
         const auto blocker = QSignalBlocker(widget);
         (widget->*func)(value);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
   }
   template<class W, class Wx, class T, class U> requires (std::is_base_of_v<Wx, W>&& std::is_convertible_v<T, U> && !std::is_reference_v<T>)
   void set_widget_property_and_block_signals(W* widget, void (Wx::* func)(T), U value) {
      auto* task = new tasks::s2m::lambda(false);
      task->handler = [widget, value, func]() {
         const auto blocker = QSignalBlocker(widget);
         (widget->*func)(value);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
   }
}