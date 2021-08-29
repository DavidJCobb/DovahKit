#include "tabbox.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/warning.h"
#include "../../core/subsystems/permissions.h"
#include "../../core/subsystems/userdata.h"
#include "../../push_native_object.h"
#include "../../send_script_task.h"
#include "../../task_reference.h"
#include "../../widget_overrides.h"
#include "../../wrapper.h"

#include "../../tasks/s2m/create_ui_widget.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/qt_alignment.h"
#include "../../api_helpers/widget_properties.h"

#include "tabbox/_util.h"
#include "tabbox/collection_tabs.h"
#include "tabbox/tab.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::ui::tabbox;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      int add_tab(lua_State* L) {
         auto&   self = get_wrapper_for_thiscall<cls>(L);
         QString name;
         if (lua_isstring(L, 2))
            name = QString::fromUtf8(lua_tostring(L, 2));
         if (!self.widget)
            return 0;
         QWidget* body = dovahscript::wrappers::ui::impl::tabbox::create_tab_widget(*(wrapped_type*)self.widget, name);
         return push_native_object(body);
      }
      int insert_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   at   = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum, 2, "integer expected");
         if (--at < 1)
            luaL_argerror(L, 2, "tabs are numbered from 1");
         QString name;
         if (lua_isstring(L, 3))
            name = QString::fromUtf8(lua_tostring(L, 3));
         //
         if (!self.widget)
            return 0;
         //
         QWidget* body = dovahscript::wrappers::ui::impl::tabbox::create_tab_widget(*(wrapped_type*)self.widget, name, at);
         return push_native_object(body);
      }
      int remove_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         QWidget* target = nullptr;
         int      isnum;
         int      at = lua_tointegerx(L, 2, &isnum);
         if (isnum) {
            cobb::lua::argcheck(L, at >= 1, 2, "tabs are numbered from 1");
            --at;
         } else {
            at = -1;
            //
            auto* arg = wrapper_from_stack<wrappers::ui::tabbox_tab>(L, 2);
            cobb::lua::argcheck(L, arg != nullptr, 2, "integer or tabbox_tab expected");
            target = arg->widget;
            if (!target)
               return 0;
         }
         //
         if (!self.widget)
            return 0;
         dovahscript::wrappers::ui::impl::tabbox::remove_tab_widget(L, *(wrapped_type*)self.widget, target, at);
         return 0;
      }
   }
   namespace _getters {
      int allow_reordering(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTabWidget::isMovable);
         lua_pushboolean(L, result);
         return 1;
      }
      int selected_index(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &QTabWidget::currentIndex);
         lua_pushinteger(L, result);
         return 1;
      }
      int selected_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QWidget* result = nullptr;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto  widget  = task_reference((wrapped_type*) self.widget);
            task->handler = [widget, &result]() {
               result = widget->currentWidget();
            };
            send_script_ui_task(*task);
            delete task;
         }
         return push_native_object(result);
      }
      int tabs(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_tabbox_tabs;
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::ui::collections::tabbox_tabs.registry_key);
      }
   }
   namespace _setters {
      int allow_reordering(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         cobb::lua::argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &QTabWidget::setMovable, value);
         return 0;
      }
      int selected_index(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int index = lua_tointegerx(L, 2, &isnum);
         cobb::lua::argcheck(L, isnum, 2, "integer expected");
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property_and_block_signals((wrapped_type*)self.widget, &QTabWidget::setCurrentIndex, index);
         return 0;
      }
      int selected_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<wrappers::ui::tabbox_tab>(L, 2);
         cobb::lua::argcheck(L, arg != nullptr, 2, "tabbox_tab expected");
         if (!self.widget || !arg->widget)
            return 0;
         {
            auto  widget  = task_reference((wrapped_type*) self.widget);
            auto  target  = task_reference(arg->widget);
            auto* task    = new tasks::s2m::ui_write_lambda(false);
            task->handler = [widget, target]() {
               const auto blocker = QSignalBlocker(widget);
               widget->setCurrentWidget(target);
            };
            send_script_ui_task(*task);
         }
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         core::subsystems::permissions::verify_ui_permissions();
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.%s.new function should not be called with a colon or passed any arguments", cls::global_name);
         //
         auto* task = new tasks::s2m::create_ui_widget<wrapped_type>();
         send_script_ui_task(*task);
         auto* created = task->created;
         delete task;
         //
         return push_native_object(created);
      }
      int is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace dovahscript::wrappers::ui {
   /*static*/ cls::method_list_t cls::metatable_methods = {
      { "add_tab",    &_methods::add_tab },
      { "insert_tab", &_methods::insert_tab },
      { "remove_tab", &_methods::remove_tab },
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "allow_reordering", &_getters::allow_reordering },
      { "selected_index",   &_getters::selected_index },
      { "selected_tab",     &_getters::selected_tab },
      { "tabs",             &_getters::tabs },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "allow_reordering", &_setters::allow_reordering },
      { "selected_index",   &_setters::selected_index },
      { "selected_tab",     &_setters::selected_tab },
   };

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      define_collection_metatable(L, wrappers::ui::collections::tabbox_tabs);
   }

   /*static*/ void cls::import_singleton(lua_State* L) {
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
   }
}