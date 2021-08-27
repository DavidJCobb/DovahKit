#pragma once
#include <type_traits>
#include "../../../../helpers/qt/traversal.h"
#include "../../../../lua.h"
#include "../../../qt/DovahscriptTabboxTab.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"

#include "../../../tasks/s2m/ui_read_lambda.h"
#include "../../../tasks/s2m/ui_write_lambda.h"

class QTabWidget;

namespace dovahscript::wrappers::ui::impl::tabbox {
   // Call directly from the script API. Configures and sends a task as appropriate.
   extern QWidget* create_tab_widget(QTabWidget& parent, const QString& text, int at = -1);

   extern void remove_tab_widget(lua_State* L, QTabWidget& parent, QWidget* body = nullptr, int at = -1);

   #pragma region Tab property accessors
   template<typename R> R get_tab_property(DovahscriptTabboxTab* widget, bool& found, R (QTabWidget::* tabbox_func)(int) const) {
      R result;
      found = false;
      {
         auto* task = new tasks::s2m::ui_read_lambda();
         auto  tab  = task_reference(widget);
         task->handler = [widget, &found, tabbox_func, &result]() {
            auto* parent = tab->containingTabWidget();
            if (!parent)
               return;
            found = true;
            result = (parent->*tabbox_func)(parent->indexOf(tab));
         };
         send_script_ui_task(*task);
         delete task;
      }
      return result;
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Use this for when 
   // the argument should be passed by reference.
   //
   template<class T> void set_tab_property(DovahscriptTabboxTab* widget, void (QTabWidget::* tabbox_func)(int, const T&), const T& value) {
      auto* task = new tasks::s2m::ui_write_lambda(false);
      auto  tab  = task_reference(widget);
      task->handler = [tab, value, tabbox_func]() {
         auto* parent = tab->containingTabWidget();
         if (!parent)
            return;
         (parent->*tabbox_func)(parent->indexOf(tab), value);
      };
      send_script_ui_task(*task);
   }

   //
   // Helper function for setting a widget property via a fire-and-forget task. Separate template 
   // types are used for the QWidget function argument (T) and for the value (U) so that you can 
   // pass in inexact matches, e.g. passing a lua_Integer value to a function that takes int.
   //
   template<class T, class U> requires (std::is_convertible_v<T, U> && !std::is_reference_v<T>) void set_tab_property(DovahscriptTabboxTab* widget, void (QTabWidget::* tabbox_func)(int, T), U value) {
      auto* task = new tasks::s2m::ui_write_lambda(false);
      auto  tab  = task_reference(widget);
      task->handler = [tab, value, tabbox_func]() {
         auto* parent = tab->containingTabWidget();
         if (!parent)
            return;
         (parent->*tabbox_func)(parent->indexOf(widget), value);
      };
      send_script_ui_task(*task);
   }
   #pragma endregion
}