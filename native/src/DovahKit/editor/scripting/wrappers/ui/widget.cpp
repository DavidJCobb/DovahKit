#include "widget.h"
#include "../../systems/editor_script_inner_core.h"
#include "../../systems/messaging.h"
#include "../../systems/permissions.h"
#include "../../systems/ui_listeners.h"
#include "../../systems/userdata.h"

#include "../../wrapper_util.h"

#include <QBoxLayout>
#include <QDialog>
#include <QFrame>
#include <QGridLayout>

#include "../../cross_thread_tasks/s2m/lambda.h"
#include "../../../../helpers/lua/qt_variant.h"

#include "helpers/widget_properties.h"

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
            DovahKitScriptVMCore::get().widget_no_longer_orphaned(child);
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
      luastackchange_t get_layout_stretch_at(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         auto* layout = self.widget->layout();
         if (!layout)
            return 0;
         if (auto* grid = qobject_cast<QGridLayout*>(layout)) {
            luaL_argcheck(L, lua_isstring (L, 2), 2, "axis name (\"row\" or \"col\" or \"column\") expected");
            luaL_argcheck(L, lua_isinteger(L, 3), 3, "index (integer) expected");
            lua_settop(L, 3);
            auto* axis  = lua_tostring(L, 2);
            int   index = lua_tointeger(L, 3);
            if (_stricmp(axis, "row") == 0) {
               lua_pushinteger(L, grid->rowStretch(index));
               return 1;
            }
            if (_stricmp(axis, "col") == 0 || _stricmp(axis, "column") == 0) {
               lua_pushinteger(L, grid->columnStretch(index));
               return 1;
            }
            luaL_error(L, "axis name \"%s\" is unrecognized", axis);
            __assume(0); // unreachable
         }
         if (auto* box = qobject_cast<QBoxLayout*>(layout)) {
            luaL_argcheck(L, lua_isinteger(L, 2), 2, "layout index (integer) expected");
            lua_settop(L, 2);
            lua_pushinteger(L, box->stretch(lua_tointeger(L, 2)));
            return 1;
         }
         return 0;
      }
      luastackchange_t on(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring  (L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring  (L, 3), 3, "listener name (string) expected");
         luaL_argcheck(L, lua_isfunction(L, 4), 4, "listener (function) expected");
         lua_settop(L, 4);
         if (!self.widget)
            return 0;
         DovahKitScriptUIListenerInterface::get().add_listener(*self.widget, lua_tostring(L, 2), lua_tostring(L, 3), 4);
         return 0;
      }
      luastackchange_t remove_child(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<cls>(L, 2);
         luaL_argcheck(L, arg != nullptr, 2, "child (widget) expected");
         __assume(arg != nullptr);
         if (!self.widget)
            return 0;
         if (arg->widget == self.widget)
            luaL_error(L, "cannot remove a widget from itself; a widget cannot be its own child");
         if (!_can_have_layout(*self.widget))
            luaL_error(L, "this widget cannot have a layout and so cannot have children either");
         if (!arg->widget)
            return 0;
         //
         auto* widget = self.widget;
         auto* child  = arg->widget;
         auto* task   = new tasks::s2m::lambda(true); // blocking
         bool  is_not_a_child = false;
         task->handler = [widget, child, &is_not_a_child]() {
            if (child->parentWidget() != widget) {
               is_not_a_child = true;
               return;
            }
            child->setParent(nullptr);
            DovahKitScriptVMCore::get().accept_new_orphaned_widget(child);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         if (is_not_a_child)
            luaL_error(L, "the argument widget isn't a child of this widget");
         //
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
      luastackchange_t set_layout_stretch_at(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         //
         QVector<QVariant> args;
         auto argcount = lua_gettop(L) - 1;
         for (int i = 0; i < argcount; ++i) {
            args.push_back(cobb::lua::to_qt_variant(L, i + 2));
         }
         //
         auto* widget = self.widget;
         auto* task   = new tasks::s2m::lambda(true); // IMPORTANT: this must be a blocking task, so that we can react to its results (e.g. report errors)
         struct {
            const char* text = nullptr;
            int         arg  = 0;
            const char* bad  = nullptr;
         } error;
         task->handler = [widget, args, &error]() {
            auto* layout = widget->layout();
            if (!layout)
               return;
            if (auto* grid = qobject_cast<QGridLayout*>(layout)) {
               if (args.size() < 3) {
                  error.text = "this widget has a grid layout, so you must pass the axis name, an index, and a stretch value";
                  return;
               }
               if (args[0].type() != QMetaType::QString) {
                  error.arg  = 2;
                  error.text = "axis name (\"row\" or \"col\" or \"column\") expected";
                  return;
               }
               if (!cobb::lua::qt_variant_is_int(args[1])) {
                  error.arg  = 3;
                  error.text = "index (integer) expected";
                  return;
               }
               if (!cobb::lua::qt_variant_is_int(args[2])) {
                  error.arg = 4;
                  error.text = "stretch (integer) expected";
                  return;
               }
               auto axis    = args[0].toString();
               auto index   = args[1].toInt() - 1;
               auto stretch = args[2].toInt();
               if (axis.compare("row", Qt::CaseInsensitive) == 0) {
                  grid->setRowStretch(index, stretch);
                  return;
               }
               if (axis.compare("col", Qt::CaseInsensitive) == 0 || axis.compare("column", Qt::CaseInsensitive) == 0) {
                  grid->setColumnStretch(index, stretch);
                  return;
               }
               error.text = "axis name \"%s\" is unrecognized";
               error.bad  = axis.toUtf8();
               return;
            }
            if (auto* box = qobject_cast<QBoxLayout*>(layout)) {
               if (!cobb::lua::qt_variant_is_int(args[0])) {
                  error.arg  = 2;
                  error.text = "index (integer) expected";
                  return;
               }
               if (!cobb::lua::qt_variant_is_int(args[1])) {
                  error.arg = 3;
                  error.text = "stretch (integer) expected";
                  return;
               }
               auto index   = args[0].toInt() - 1;
               auto stretch = args[1].toInt();
               box->setStretch(index, stretch);
               return;
            }
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
         //
         if (error.text) {
            if (error.arg)
               luaL_argcheck(L, false, error.arg, error.text);
            luaL_error(L, error.text, error.bad);
         }
         return 0;
      }
   }
   namespace _getters {
      luastackchange_t enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::isEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      luastackchange_t layout_margins(lua_State* L) {
         lua_settop(L, 1);
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         if (!_can_have_layout(*self.widget))
            return 0;
         QMargins result;
         {
            auto* widget = (wrapped_type*)self.widget;
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               auto* layout = widget->layout();
               if (layout)
                  result = layout->contentsMargins();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_createtable(L, 4, 4);
         int value;
         //
         value = result.top();
         lua_pushinteger(L, value);
         lua_rawseti(L, 2, 1);
         lua_pushinteger(L, value);
         lua_setfield(L, 2, "top");
         //
         value = result.right();
         lua_pushinteger(L, value);
         lua_rawseti(L, 2, 2);
         lua_pushinteger(L, value);
         lua_setfield(L, 2, "right");
         //
         value = result.bottom();
         lua_pushinteger(L, value);
         lua_rawseti(L, 2, 3);
         lua_pushinteger(L, value);
         lua_setfield(L, 2, "bottom");
         //
         value = result.left();
         lua_pushinteger(L, value);
         lua_rawseti(L, 2, 4);
         lua_pushinteger(L, value);
         lua_setfield(L, 2, "left");
         //
         return 1;
      }
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::objectName);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::toolTip);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      luastackchange_t whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = editor_script::helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::whatsThis);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setEnabled, value);
         return 0;
      }
      luastackchange_t layout_margins(lua_State* L) {
         //
         // Accepts any of the following kinds of values:
         // 
         //    my_widget.layout_margins = 5
         //    my_widget.layout_margins = { 5 }
         //    my_widget.layout_margins = { 1, 2, 3, 4 }
         //    my_widget.layout_margins = { top = 1, right = 2, bottom = 3, left = 4 }
         //
         // When fewer than four values are supplied, follows the same rules as CSS's "margin" 
         // property. Non-integer numeric values are truncated to integers. Non-numeric values 
         // are treated as "unchanged," but supplying a table that offers no valid numbers is 
         // an error. If both named keys and numeric keys are provided, only the latter are 
         // used.
         //
         // Supplying alternate values (e.g. an integer; an "incomplete" table) does not change 
         // the return type of the corresponding getter:
         //
         //    my_widget.layout_margins = 5
         //    local now = my_widget.layout_margins -- table
         //
         lua_settop(L, 2);
         //
         constexpr int unchanged  = -1;
         constexpr int offset_arg = 2;
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int top    = unchanged;
         int bottom = unchanged;
         int left   = unchanged;
         int right  = unchanged;
         int unchanged_count = 4;
         if (lua_istable(L, 2)) {
            lua_len(L, 2);
            int argcount = 0;
            if (lua_isnumber(L, 3))
               argcount = std::min(4, (int)lua_tonumber(L, 3));
            lua_pop(L, 1);
            if (argcount) {
               //
               // The user passed in an array of numbers.
               //
               for (int i = 1; i <= 4; ++i) {
                  lua_geti(L, 2, i);
                  if (!lua_isnumber(L, -1)) {
                     lua_pop(L, 1);
                     lua_pushinteger(L, unchanged);
                  } else if (lua_tonumber(L, -1) < 0) {
                     lua_pop(L, 1);
                     lua_pushinteger(L, 0);
                  }
               }
               switch (argcount) {
                  case 1:
                     top = bottom = left = right = lua_tonumber(L, offset_arg + 1);
                  case 2:
                     top = bottom = lua_tonumber(L, offset_arg + 1);
                     left = right = lua_tonumber(L, offset_arg + 2);
                     break;
                  case 3:
                     top    = lua_tonumber(L, offset_arg + 1);
                     left   = lua_tonumber(L, offset_arg + 2);
                     right  = lua_tonumber(L, offset_arg + 2);
                     bottom = lua_tonumber(L, offset_arg + 3);
                     break;
                  case 4:
                     top    = lua_tonumber(L, offset_arg + 1);
                     right  = lua_tonumber(L, offset_arg + 2);
                     bottom = lua_tonumber(L, offset_arg + 3);
                     left   = lua_tonumber(L, offset_arg + 4);
                     break;
               }
               unchanged_count = 0;
            } else {
               //
               // The user passed in an arbitrary table. Check if it has the needed fields.
               //
               lua_getfield(L, 2, "top");
               lua_getfield(L, 2, "left");
               lua_getfield(L, 2, "right");
               lua_getfield(L, 2, "bottom");
               if (lua_isnumber(L, 3)) {
                  --unchanged_count;
                  top = lua_tonumber(L, 3);
                  if (top < 0)
                     top = 0;
               }
               if (lua_isnumber(L, 4)) {
                  --unchanged_count;
                  left = lua_tonumber(L, 4);
                  if (left < 0)
                     left = 0;
               }
               if (lua_isnumber(L, 5)) {
                  --unchanged_count;
                  right = lua_tonumber(L, 5);
                  if (right < 0)
                     right = 0;
               }
               if (lua_isnumber(L, 6)) {
                  --unchanged_count;
                  bottom = lua_tonumber(L, 6);
                  if (bottom < 0)
                     bottom = 0;
               }
               lua_pop(L, 4);
               //
               if (unchanged_count == 4)
                  luaL_error(L, "the supplied table didn't specify any margins");
            }
         } else {
            int isnum;
            top = lua_tointegerx(L, 2, &isnum);
            luaL_argcheck(L, isnum, 2, "integer or table expected");
            if (top < 0)
               top = 0;
            left = right = bottom = top;
            unchanged_count = 0;
         }
         //
         if (!self.widget)
            return 0;
         if (!_can_have_layout(*self.widget))
            luaL_error(L, "this widget cannot have a layout");
         auto* widget = self.widget;
         auto* task   = new tasks::s2m::lambda(false);
         task->handler = [widget, top, right, bottom, left, unchanged_count, unchanged]() mutable {
            auto* layout = widget->layout();
            if (!layout)
               return;
            if (unchanged_count) {
               auto old = layout->contentsMargins();
               if (top == unchanged)
                  top = old.top();
               if (left == unchanged)
                  left = old.left();
               if (right == unchanged)
                  right = old.right();
               if (bottom == unchanged)
                  bottom = old.bottom();
            }
            layout->setContentsMargins(left, top, right, bottom);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setObjectName, value);
         return 0;
      }
      luastackchange_t tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "tooltip text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setToolTip, value);
         return 0;
      }
      luastackchange_t whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         editor_script::helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setWhatsThis, value);
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
      { "add_child",             &_methods::add_child },
      { "can_have_layout",       &_methods::can_have_layout },
      { "get_layout_stretch_at", &_methods::get_layout_stretch_at },
      { "on",                    &_methods::on },
      { "remove_child",          &_methods::remove_child },
      { "remove_event_listener", &_methods::remove_event_listener },
      { "set_layout",            &_methods::set_layout },
      { "set_layout_stretch_at", &_methods::set_layout_stretch_at },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "enabled",        &_getters::enabled },
      { "layout_margins", &_getters::layout_margins },
      { "name",           &_getters::name },
      { "tooltip",        &_getters::tooltip },
      { "whats_this",     &_getters::whats_this },
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "enabled",        &_setters::enabled },
      { "layout_margins", &_setters::layout_margins },
      { "name",           &_setters::name },
      { "tooltip",        &_setters::tooltip },
      { "whats_this",     &_setters::whats_this },
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