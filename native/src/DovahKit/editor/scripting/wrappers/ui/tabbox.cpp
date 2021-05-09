#include "tabbox.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../ui/util/alignment.h"

#include "helpers/widget_properties.h"

#include "tabbox/tab.h"

#pragma region Collection: "tabs"
namespace {
   using namespace editor_script;

   namespace _collections::items {
      using cls = wrappers::ui::tabbox;

      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, cls::tab_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", cls::tab_collection_key);
         }
         if (self->widget == nullptr) {
            luaL_error(L, "function called with zombie self (expected %s)", cls::tab_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* widget = (cls::wrapped_type*) self.widget;
         int result;
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               result = widget->count();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self   = get_collection_wrapper(L);
         auto* widget = (cls::wrapped_type*)self.widget;
         auto  i      = lua_tointeger(L, 2) - 1; // lua indices start from one, not zero
         if (i < 0 || i >= widget->count())
            return 0;
         QWidget* result = nullptr;
         {
            auto* task = new tasks::s2m::lambda(true);
            task->handler = [&self, i, &result]() {
               auto* widget = (cls::wrapped_type*) self.widget;
               result = widget->widget(i);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!result)
            return 0;
         wrapper out;
         out.widget = result;
         out.type   = wrapper_type::ui;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::ui::tabbox_tab::metatable_key);
      }
   }
}
#pragma endregion

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::tabbox;
   using wrapped_type = cls::wrapped_type;

   namespace _methods {
      luastackchange_t add_tab(lua_State* L) {
         auto&   self = get_wrapper_for_thiscall<cls>(L);
         QString name;
         if (lua_isstring(L, 2))
            name = QString::fromUtf8(lua_tostring(L, 2));
         if (!self.widget)
            return 0;
         QWidget* body = nullptr;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [widget, &name, &body]() {
               body = new QWidget;
               DovahKitScriptVMCore::get().set_up_new_scripted_widget(body);
               override_widget_metatable(body, wrappers::ui::tabbox_tab::metatable_key);
               widget->addTab(body, name);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         wrapper out;
         out.widget = body;
         out.type   = wrapper_type::ui;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::ui::tabbox_tab::metatable_key);
      }
      luastackchange_t insert_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int   isnum;
         int   at   = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
         if (--at < 1)
            luaL_argerror(L, 2, "tabs are numbered from 1");
         QString name;
         if (lua_isstring(L, 3))
            name = QString::fromUtf8(lua_tostring(L, 3));
         //
         if (!self.widget)
            return 0;
         //
         QWidget* body = nullptr;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [widget, at, &name, &body]() {
               body = new QWidget;
               DovahKitScriptVMCore::get().set_up_new_scripted_widget(body);
               override_widget_metatable(body, wrappers::ui::tabbox_tab::metatable_key);
               widget->insertTab(at, body, name);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         wrapper out;
         out.widget = body;
         out.type   = wrapper_type::ui;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::ui::tabbox_tab::metatable_key);
      }
      luastackchange_t remove_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         //
         QWidget* target = nullptr;
         int isnum;
         int at = lua_tointegerx(L, 2, &isnum);
         if (isnum) {
            --at;
            if (at < 1)
               luaL_argerror(L, 2, "tabs are numbered from 1");
         } else {
            auto* arg = wrapper_from_stack<wrappers::ui::tabbox_tab>(L, 2);
            if (arg == nullptr) {
               luaL_argerror(L, 2, "integer or tabbox_tab expected");
               return 0;
            }
            target = arg->widget;
            if (!target)
               return 0;
         }
         //
         if (!self.widget)
            return 0;
         //
         QWidget* body = nullptr;
         {
            auto* widget  = (wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::lambda(true);
            task->handler = [widget, at, target]() {
               widget->removeTab(target ? widget->indexOf(target) : at);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t allow_reordering(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTabWidget::isMovable);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t selected_index(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &QTabWidget::currentIndex);
         lua_pushinteger(L, result);
         return 1;
      }
      luastackchange_t selected_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QWidget* result = nullptr;
         {
            auto* task    = new tasks::s2m::ui_read_lambda();
            auto* widget  = (cls::wrapped_type*) self.widget;
            task->handler = [widget, &result]() {
               result = widget->currentWidget();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (!result)
            return 0;
         wrapper out;
         out.widget = result;
         out.type   = wrapper_type::ui;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, wrappers::ui::tabbox_tab::metatable_key);
      }
      luastackchange_t tabs(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         wrapper out = self;
         out.parts[0].signature = wrapper_part_types::ui_tabbox_tabs;
         out.is_collection = true;
         return DovahKitScriptVMUserdataInterface::get().push(L, out, cls::tab_collection_key);
      }
   }
   namespace _setters {
      luastackchange_t allow_reordering(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &QTabWidget::setMovable, value);
         return 0;
      }
      luastackchange_t selected_index(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int index = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
         if (!self.widget)
            return 0;
         {
            auto* widget  = (cls::wrapped_type*) self.widget;
            auto* task    = new tasks::s2m::lambda(false);
            task->handler = [widget, index]() {
               const auto blocker = QSignalBlocker(widget);
               widget->setCurrentIndex(index);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
         }
         return 0;
      }
      luastackchange_t selected_tab(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<wrappers::ui::tabbox_tab>(L, 2);
         luaL_argcheck(L, arg != nullptr, 2, "tabbox_tab expected");
         if (!self.widget || !arg->widget)
            return 0;
         {
            auto* widget  = (cls::wrapped_type*) self.widget;
            auto* target  = arg->widget;
            auto* task    = new tasks::s2m::lambda(false);
            task->handler = [widget, target]() {
               const auto blocker = QSignalBlocker(widget);
               widget->setCurrentWidget(target);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
         }
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.tabbox.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type();
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         wrapper out;
         auto* mt = wrap_widget(out, created);
         return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
      }
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
      { "add_tab",    &_methods::add_tab },
      { "insert_tab", &_methods::insert_tab },
      { "remove_tab", &_methods::remove_tab },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "allow_reordering", &_getters::allow_reordering },
      { "selected_index",   &_getters::selected_index },
      { "selected_tab",     &_getters::selected_tab },
      { "tabs",             &_getters::tabs },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "allow_reordering", &_setters::allow_reordering },
      { "selected_index",   &_setters::selected_index },
      { "selected_tab",     &_setters::selected_tab },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      editor_script::define_class(L, metatable_key, nullptr, metatable_methods);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::new_);
      lua_setfield     (L, -2, "new");
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
   }
}