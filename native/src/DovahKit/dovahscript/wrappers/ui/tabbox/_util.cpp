#include "_util.h"
#include <QTabWidget>
#include "../../../../helpers/lua/error.h"
#include "../../../core/subsystems/coordinator.h"
#include "../../../core/subsystems/lifetime.h"
#include "../../../qt/DovahscriptTabboxTab.h"
#include "../../../send_script_task.h"
#include "../../../task_reference.h"
#include "../../../widget_overrides.h"

#include "../../../tasks/s2m/ui_write_lambda.h"

namespace dovahscript::wrappers::ui::impl::tabbox {
   extern QWidget* create_tab_widget(QTabWidget& parent, const QString& name, int at) {
      QWidget* body = nullptr;
      {
         auto  widget  = task_reference(&parent);
         auto* task    = new tasks::s2m::ui_write_lambda(true);
         task->handler = [widget, at, &name, &body]() {
            body = new DovahscriptTabboxTab;
            core::subsystems::coordinator::get().set_up_widget(*body);
            core::subsystems::lifetime::get().on_hierarchy_item_created(*body);
            set_widget_forced_parent(body, widget);
            if (at >= 0)
               widget->insertTab(at, body, name);
            else
               widget->addTab(body, name);
         };
         send_script_ui_task(*task);
         delete task;
      }
      return body;
   }

   extern void remove_tab_widget(lua_State* L, QTabWidget& parent, QWidget* target, int at) {
      bool err_out_of_bounds = false;
      //
      auto  widget = task_reference(&parent);
      auto* task   = new tasks::s2m::ui_write_lambda(true);
      task->handler = [widget, at, target, &err_out_of_bounds]() mutable {
         if (!target) {
            target = widget->widget(at);
            if (!target) {
               err_out_of_bounds = true;
               return;
            }
         } else {
            at = widget->indexOf(target);
         }
         widget->removeTab(target ? widget->indexOf(target) : at);
         target->setParent(nullptr);
         set_widget_forced_parent(target, nullptr);
         core::subsystems::lifetime::get().on_hierarchy_item_parent_changed(target, widget);
      };
      send_script_ui_task(*task);
      delete task;
      //
      if (err_out_of_bounds)
         cobb::lua::error(L, "tab index %d is out of bounds", at + 1);
   }
}