#include "widget.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include <QBoxLayout>
#include <QDialog>
#include <QFrame>
#include <QGridLayout>

#include "../../cross_thread_tasks/s2m/lambda.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::widget;
   using wrapped_type = cls::wrapped_type;

   bool _can_have_layout(QWidget& widget) {
      if (widget.metaObject() == &QWidget::staticMetaObject) // is QWidget and NOT a subclass
         return true;
      if (qobject_cast<QDialog*>(&widget))
         return true;
      if (qobject_cast<QFrame*>(&widget))
         return true;
      return false;
   }

   struct _layout_type {
      const char* name;
      bool is_box  = false;
      bool is_grid = false;
      QBoxLayout::Direction box_direction = QBoxLayout::Direction::LeftToRight;
   };

   std::array _layout_types = {
      _layout_type{ "none", false, false },
      _layout_type{ "grid", false, true },
      _layout_type{ "h",    true,  false, QBoxLayout::Direction::LeftToRight },
      _layout_type{ "v",    true,  false, QBoxLayout::Direction::TopToBottom },
      _layout_type{ "ltr",  true,  false, QBoxLayout::Direction::LeftToRight },
      _layout_type{ "rtl",  true,  false, QBoxLayout::Direction::RightToLeft },
      _layout_type{ "down", true,  false, QBoxLayout::Direction::TopToBottom },
      _layout_type{ "up",   true,  false, QBoxLayout::Direction::BottomToTop },
   };

   namespace _methods {
      luastackchange_t add_child(lua_State* L) {
         int   argcount = lua_gettop(L);
         auto& self     = get_wrapper_for_thiscall<cls>(L);
         auto* arg      = wrapper_from_stack<cls>(L, 2);
         luaL_argcheck(L, arg != nullptr, 2, "child (widget) expected");
         __assume(arg != nullptr);
         if (!self.widget)
            return 0;
         if (arg->widget == self.widget)
            luaL_error(L, "a widget cannot be its own child");
         if (!_can_have_layout(*self.widget))
            luaL_error(L, "this widget cannot have a layout and so cannot have children either");
         if (!arg->widget)
            return 0;
         //
         int row     = 0; // or (index) for boxes
         int col     = 0;
         int rowspan = 1;
         int colspan = 1;
         //
         if (lua_isnumber(L, 3))
            row     = lua_tonumber(L, 3);
         if (lua_isnumber(L, 4))
            col     = lua_tonumber(L, 4);
         if (lua_isnumber(L, 5))
            rowspan = lua_tonumber(L, 5);
         if (lua_isnumber(L, 6))
            colspan = lua_tonumber(L, 6);
         --row; // Lua one-indexed -> C zero-indexed
         --col; // Lua one-indexed -> C zero-indexed
         //
         auto* widget = self.widget;
         auto* child  = arg->widget;
         auto* task   = new tasks::s2m::lambda(false);
         task->handler = [widget, child, row, col, rowspan, colspan]() {
            auto* layout = widget->layout();
            if (!layout) {
               child->setParent(widget);
            } else if (auto* grid = qobject_cast<QGridLayout*>(layout)) {
               if (row >= 0 && col >= 0) {
                  grid->addWidget(child, row, col, rowspan, colspan);
               } else {
                  grid->addWidget(child);
               }
            } else if (auto* box = qobject_cast<QBoxLayout*>(layout)) {
               if (row >= 0) {
                  box->insertWidget(row, child);
               } else {
                  box->addWidget(child);
               }
            }
            DovahKitScriptVM::get().widget_no_longer_orphaned(child);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t can_have_layout(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         lua_pushboolean(L, _can_have_layout(*self.widget));
         return 1;
      }
      luastackchange_t on(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         luaL_argcheck(L, lua_isfunction(L, 4), 4, "listener (function) expected");
         lua_settop(L, 4);
         if (!self.widget)
            return 0;
         DovahKitScriptUIListenerInterface::get().add_listener(*self.widget, lua_tostring(L, 2), lua_tostring(L, 3), 4);
         return 0;
      }
      luastackchange_t remove_event_listener(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         lua_settop(L, 3);
         if (!lua_isnoneornil(L, 3))
            luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         if (!self.widget)
            return 0;
         DovahKitScriptUIListenerInterface::get().remove_listener(*self.widget, lua_tostring(L, 2), lua_tostring(L, 3));
         return 0;
      }
      luastackchange_t set_layout(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "layout type (string) expected");
         lua_settop(L, 2);
         if (!self.widget)
            return 0;
         if (!_can_have_layout(*self.widget))
            luaL_error(L, "this widget cannot have a layout");
         //
         _layout_type* known_type = nullptr;
         auto*         type_name  = lua_tostring(L, 2);
         for (auto& t : _layout_types) {
            if (_stricmp(type_name, t.name) == 0) {
               known_type = &t;
               break;
            }
         }
         if (!known_type) {
            luaL_error(L, "layout type \"%s\" is unrecognized", type_name);
            __assume(0); // unreachable
         }
         auto* widget = self.widget;
         auto* task   = new tasks::s2m::lambda(false);
         task->handler = [widget, known_type]() {
            auto* old = widget->layout();
            if (known_type->is_grid) {
               if (qobject_cast<QGridLayout*>(old))
                  return;
               widget->setLayout(nullptr);
               widget->setLayout(new QGridLayout(widget));
               return;
            } else if (known_type->is_box) {
               auto dir = known_type->box_direction;
               if (auto* box = qobject_cast<QBoxLayout*>(old)) {
                  box->setDirection(dir);
               } else {
                  widget->setLayout(nullptr);
                  widget->setLayout(new QBoxLayout(dir, widget));
               }
               return;
            } else {
               widget->setLayout(nullptr); // "none"
            }
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result;
         {
            auto* widget  = self.widget;
            auto* task    = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() { result = widget->isEnabled(); };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, result);
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto* widget  = (wrapped_type*) self.widget;
         auto* task    = new tasks::s2m::lambda(false);
         bool  value   = lua_toboolean(L, 2);
         task->handler = [widget, value]() { widget->setEnabled(value); };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
   }

   namespace _singleton_functions {
      luastackchange_t new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            luaL_error(L, "the ui.widget.new function should not be called with a colon or passed any arguments");
         //
         DovahKitScriptVMPermissionInterface::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type;
            DovahKitScriptVM::get().accept_new_orphaned_widget(created);
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
      { "add_child",             &_methods::add_child },
      { "can_have_layout",       &_methods::can_have_layout },
      { "on",                    &_methods::on },
      { "remove_event_listener", &_methods::remove_event_listener },
      { "set_layout",            &_methods::set_layout },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "enabled", &_getters::enabled },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "enabled", &_setters::enabled },
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