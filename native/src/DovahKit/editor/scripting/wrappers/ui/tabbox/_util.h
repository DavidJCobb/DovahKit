#pragma once
#include <type_traits>
#include "../../../../../helpers/qt/traversal.h"

namespace editor_script::util::tabbox {
   template<class W, typename R> R get_tab_property(W* widget, bool& found, R (QTabWidget::* tabbox_func)(int) const) {
      R result;
      found = false;
      {
         auto* task = new tasks::s2m::ui_read_lambda();
         task->handler = [widget, &found, tabbox_func, &result]() {
            auto* parent = (QTabWidget*) cobb::qt::nearest_widget_of_type(widget, QTabWidget::staticMetaObject);
            if (!parent)
               return;
            found = true;
            result = (parent->*tabbox_func)(parent->indexOf(widget));
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
      }
      return result;
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Use this for when 
   // the argument should be passed by reference.
   //
   template<class W, class T> void set_tab_property(W* widget, void (QTabWidget::* tabbox_func)(int, const T&), const T& value) {
      auto* task = new tasks::s2m::lambda(false);
      task->handler = [widget, value, tabbox_func]() {
         auto* parent = (QTabWidget*)cobb::qt::nearest_widget_of_type(widget, QTabWidget::staticMetaObject);
         if (!parent)
            return;
         (parent->*tabbox_func)(parent->indexOf(widget), value);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Separate template 
   // types are used for the QWidget function argument (T) and for the value (U) so that you can 
   // pass in inexact matches, e.g. passing a lua_Integer value to a function that takes int.
   //
   template<class W, class T, class U> requires (std::is_convertible_v<T, U> && !std::is_reference_v<T>) void set_tab_property(W* widget, void (QTabWidget::* tabbox_func)(int, T), U value) {
      auto* task = new tasks::s2m::lambda(false);
      task->handler = [widget, value, tabbox_func]() {
         auto* parent = (QTabWidget*)cobb::qt::nearest_widget_of_type(widget, QTabWidget::staticMetaObject);
         if (!parent)
            return;
         (parent->*tabbox_func)(parent->indexOf(widget), value);
      };
      DovahKitScriptVMUITaskConduit::get().send_message(*task);
   }
}