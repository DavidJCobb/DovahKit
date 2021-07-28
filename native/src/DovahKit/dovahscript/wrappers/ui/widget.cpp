#include "widget.h"
#include "../../../helpers/lua/error.h"
#include "../../../helpers/lua/qt_variant.h"
#include "../../../helpers/qt/layout.h"
#include "../../push_native_object.h"
#include "../../task_reference.h"
#include "../../wrapper.h"
#include "../../core/subsystems/coordinator.h"
#include "../../core/subsystems/events.h"
#include "../../core/subsystems/lifetime.h"
#include "../../core/subsystems/permissions.h"

#include "../../tasks/s2m/lambda.h"
#include "../../tasks/s2m/ui_read_lambda.h"
#include "../../tasks/s2m/ui_write_lambda.h"

#include "../../api_helpers/widget_properties.h"

#include <QBoxLayout>
#include <QDialog>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>

namespace {
   using namespace dovahscript;
   using cls = wrappers::ui::widget;
   using wrapped_type = cls::wrapped_type;

   bool _can_have_layout(QWidget& widget) {
      if (widget.metaObject() == &QWidget::staticMetaObject) // is QWidget and NOT a subclass
         return true;
      if (qobject_cast<QDialog*>(&widget))
         return true;
      if (qobject_cast<QFrame*>(&widget))
         return true;
      if (qobject_cast<QGroupBox*>(&widget))
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
      int add_child(lua_State* L) {
         int   argcount = lua_gettop(L);
         auto& self     = get_wrapper_for_thiscall<cls>(L);
         auto* arg      = wrapper_from_stack<cls>(L, 2);
         cobb::lua::argcheck(L, arg != nullptr, 2, "child (widget) expected");
         if (!self.widget)
            return 0;
         if (arg->widget == self.widget)
            cobb::lua::error(L, "a widget cannot be its own child");
         if (!_can_have_layout(*self.widget))
            cobb::lua::error(L, "this widget cannot have a layout and so cannot have children either");
         if (!arg->widget)
            return 0;
         {
            auto data = arg->widget->property("Lua widget forced parent");
            if (data.isValid())
               cobb::lua::argerror(L, 2, "the desired child widget cannot have its parent changed");
         }
         //
         int row     = 0; // or (index) for boxes
         int col     = 0;
         int rowspan = 1;
         int colspan = 1;
         //
         int isnum;
         if (lua_isnumber(L, 3)) {
            row = lua_tointegerx(L, 3, &isnum);
            luaL_argcheck(L, isnum,   3, "positions in a layout, if specified, must be integers");
            luaL_argcheck(L, row > 0, 3, "positions in a layout, if specified, must be greater than zero");
         }
         if (lua_isnumber(L, 4)) {
            col = lua_tointegerx(L, 4, &isnum);
            luaL_argcheck(L, isnum,   4, "positions in a layout, if specified, must be integers");
            luaL_argcheck(L, col > 0, 4, "positions in a layout, if specified, must be greater than zero");
         }
         if (lua_isnumber(L, 5)) {
            rowspan = lua_tointegerx(L, 5, &isnum);
            luaL_argcheck(L, isnum,       5, "the row span, if specified, must be an integer");
            luaL_argcheck(L, rowspan > 0, 5, "the row span, if specified, must be greater than zero");
         }
         if (lua_isnumber(L, 6)) {
            colspan = lua_tointegerx(L, 6, &isnum);
            luaL_argcheck(L, isnum,       6, "the column span, if specified, must be an integer");
            luaL_argcheck(L, colspan > 0, 6, "the column span, if specified, must be greater than zero");
         }
         --row; // Lua one-indexed -> C zero-indexed
         --col; // Lua one-indexed -> C zero-indexed
         //
         task_reference widget = self.widget;
         task_reference child  = arg->widget;
         auto* task = new tasks::s2m::ui_write_lambda(false);
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
            core::subsystems::lifetime::get().on_hierarchy_item_adopted(child);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int add_spacer(lua_State* L) {
         enum class _spacer_axis {
            h,
            v,
            both,
         };
         //
         int   argcount = lua_gettop(L);
         auto& self     = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         if (!_can_have_layout(*self.widget))
            cobb::lua::error(L, "this widget cannot have a layout and so cannot have children either");
         //
         _spacer_axis axis = _spacer_axis::both;
         luaL_argcheck(L, lua_isstring(L, 2), 2, "spacer axis (string) expected");
         {
            const char* v = lua_tostring(L, 2);
            if (_stricmp(v, "h") == 0) {
               axis = _spacer_axis::h;
            } else if (_stricmp(v, "horizontal") == 0) {
               axis = _spacer_axis::h;
            } else if (_stricmp(v, "v") == 0) {
               axis = _spacer_axis::v;
            } else if (_stricmp(v, "vertical") == 0) {
               axis = _spacer_axis::v;
            } else if (_stricmp(v, "both") == 0) {
               axis = _spacer_axis::both;
            } else {
               cobb::lua::argerror(L, 2, "unrecognized spacer axis (allowed: \"h\", \"horizontal\", \"v\", \"vertical\", \"both\")");
            }
         }
         //
         int row     = 0; // or (index) for boxes
         int col     = 0;
         int rowspan = 1;
         int colspan = 1;
         //
         int isnum;
         if (lua_isnumber(L, 3)) {
            row = lua_tointegerx(L, 3, &isnum);
            luaL_argcheck(L, isnum,   3, "positions in a layout, if specified, must be integers");
            luaL_argcheck(L, row > 0, 3, "positions in a layout, if specified, must be integers");
         }
         if (lua_isnumber(L, 4)) {
            col = lua_tointegerx(L, 4, &isnum);
            luaL_argcheck(L, isnum,   4, "positions in a layout, if specified, must be integers");
            luaL_argcheck(L, col > 0, 4, "positions in a layout, if specified, must be integers");
         }
         if (lua_isnumber(L, 5)) {
            rowspan = lua_tointegerx(L, 5, &isnum);
            luaL_argcheck(L, isnum,       5, "the row span, if specified, must be an integer");
            luaL_argcheck(L, rowspan > 0, 5, "the row span, if specified, must be greater than zero");
         }
         if (lua_isnumber(L, 6)) {
            colspan = lua_tointegerx(L, 6, &isnum);
            luaL_argcheck(L, isnum,       6, "the column span, if specified, must be an integer");
            luaL_argcheck(L, colspan > 0, 6, "the column span, if specified, must be greater than zero");
         }
         --row; // Lua one-indexed -> C zero-indexed
         --col; // Lua one-indexed -> C zero-indexed
         //
         const QMetaObject* layout_mt = nullptr;
         task_reference     widget    = self.widget;
         auto* task = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, row, col, rowspan, colspan, axis, &layout_mt]() {
            auto* layout = widget->layout();
            if (!layout)
               return;
            layout_mt = layout->metaObject();
            //
            QSpacerItem* child = nullptr;
            if (auto* grid = qobject_cast<QGridLayout*>(layout)) {
               child = new QSpacerItem(0, 0);
               if (row >= 0 && col >= 0) {
                  grid->addItem(child, row, col, rowspan, colspan);
               }
            } else if (auto* box = qobject_cast<QBoxLayout*>(layout)) {
               child = new QSpacerItem(0, 0);
               if (row >= 0) {
                  box->insertSpacerItem(row, child);
               } else {
                  box->addSpacerItem(child);
               }
            }
            //
            if (child) {
               switch (axis) {
                  case _spacer_axis::h:
                     child->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
                     break;
                  case _spacer_axis::v:
                     child->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
                     break;
                  case _spacer_axis::both:
                     child->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
                     break;
               }
               layout->invalidate();
            }
         };
         //
         if (layout_mt == &QGridLayout::staticMetaObject) {
            cobb::lua::argcheck(L, row >= 0, 2, "(grid layout) you must specify a row number");
            cobb::lua::argcheck(L, col >= 0, 2, "(grid layout) you must specify a column number");
         }
         //
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int can_have_layout(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         lua_settop(L, 1);
         if (!self.widget)
            return 0;
         lua_pushboolean(L, _can_have_layout(*self.widget));
         return 1;
      }
      int get_layout_stretch_at(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QVariant arg_a = cobb::lua::to_qt_variant(L, 2);
         QVariant arg_b = cobb::lua::to_qt_variant(L, 3);
         //
         struct {
            std::string text;
            int arg = 0;
         } error;
         int result = -1;
         {
            task_reference widget = (wrapped_type*)self.widget;
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &arg_a, &arg_b, &result, &error]() {
               auto* layout = widget->layout();
               if (!layout)
                  return;
               if (auto* grid = qobject_cast<QGridLayout*>(layout)) {
                  if (!arg_a.type() == QMetaType::QString) {
                     error.text = "axis name (\"row\" or \"col\" or \"column\") expected";
                     error.arg  = 2;
                     return;
                  }
                  if (!arg_b.type() == QMetaType::Int) {
                     error.text = "index (integer) expected";
                     error.arg  = 3;
                     return;
                  }
                  auto axis  = arg_a.value<QString>();
                  int  index = arg_b.value<int>();
                  if (axis.compare("row", Qt::CaseInsensitive) == 0) {
                     result = grid->rowStretch(index);
                     return;
                  }
                  if (axis.compare("col", Qt::CaseInsensitive) == 0 || axis.compare("column", Qt::CaseInsensitive) == 0) {
                     result = grid->columnStretch(index);
                     return;
                  }
                  error.text  = "axis name \"";
                  error.text += axis.toUtf8();
                  error.text += "\" is unrecognized";
                  return;
               }
               if (auto* box = qobject_cast<QBoxLayout*>(layout)) {
                  if (!arg_a.type() == QMetaType::Int) {
                     error.text = "index (integer) expected";
                     error.arg  = 2;
                     return;
                  }
                  result = box->stretch(arg_a.value<int>());
                  return;
               }
               result = 0;
               return;
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
            delete task;
         }
         if (result < 0) {
            if (error.arg > 0)
               cobb::lua::argerror(L, error.arg, error.text.c_str());
            cobb::lua::error(L, error.text.c_str());
         }
         lua_pushinteger(L, result);
         return 1;
      }
      int on(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring  (L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring  (L, 3), 3, "listener name (string) expected");
         luaL_argcheck(L, lua_isfunction(L, 4), 4, "listener (function) expected");
         lua_settop(L, 4);
         if (!self.widget)
            return 0;
         core::subsystems::events::get().add_listener(*self.widget, lua_tostring(L, 2), lua_tostring(L, 3), 4);
         return 0;
      }
      int remove_child(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* arg  = wrapper_from_stack<cls>(L, 2);
         cobb::lua::argcheck(L, arg != nullptr, 2, "child (widget) expected");
         if (!self.widget)
            return 0;
         if (arg->widget == self.widget)
            cobb::lua::error(L, "cannot remove a widget from itself; a widget cannot be its own child");
         if (!_can_have_layout(*self.widget))
            cobb::lua::error(L, "this widget cannot have a layout and so cannot have children either");
         if (!arg->widget)
            return 0;
         {
            auto data = arg->widget->property("Lua widget forced parent");
            if (data.isValid())
               cobb::lua::argerror(L, 2, "the specified child widget cannot have its parent changed");
         }
         //
         bool  is_not_a_child = false;
         {
            task_reference widget = self.widget;
            task_reference child  = arg->widget;
            //
            auto* task = new tasks::s2m::ui_write_lambda(true); // blocking
            task->handler = [widget, child, &is_not_a_child]() {
               if (child->parentWidget() != widget) {
                  is_not_a_child = true;
                  return;
               }
               child->setParent(nullptr);
               core::subsystems::lifetime::get().on_hierarchy_item_orphaned(child);
            };
            core::subsystems::coordinator::get().send_ui_write_task(*task);
            delete task;
         }
         if (is_not_a_child)
            cobb::lua::error(L, "the argument widget isn't a child of this widget");
         //
         return 0;
      }
      int remove_event_listener(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "event name (string) expected");
         luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         lua_settop(L, 3);
         if (!lua_isnoneornil(L, 3))
            luaL_argcheck(L, lua_isstring(L, 3), 3, "listener name (string) expected");
         if (!self.widget)
            return 0;
         core::subsystems::events::get().remove_listener(*self.widget, lua_tostring(L, 2), lua_tostring(L, 3));
         return 0;
      }
      int set_layout(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "layout type (string) expected");
         lua_settop(L, 2);
         if (!self.widget)
            return 0;
         if (!_can_have_layout(*self.widget))
            cobb::lua::error(L, "this widget cannot have a layout");
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
            cobb::lua::error(L, "layout type \"%s\" is unrecognized", type_name);
         }
         //
         task_reference widget = self.widget;
         auto* task = new tasks::s2m::ui_write_lambda(false);
         task->handler = [widget, known_type]() {
            auto* old = widget->layout();
            if (known_type->is_grid) {
               if (qobject_cast<QGridLayout*>(old))
                  return;
               widget->setLayout(new QGridLayout(widget));
               return;
            } else if (known_type->is_box) {
               auto dir = known_type->box_direction;
               if (auto* box = qobject_cast<QBoxLayout*>(old)) {
                  box->setDirection(dir);
               } else {
                  widget->setLayout(new QBoxLayout(dir, widget));
               }
               return;
            } else {
               cobb::qt::remove_layout(widget); // "none"
            }
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int set_layout_stretch_at(lua_State* L) {
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
         task_reference widget = self.widget;
         auto* task = new tasks::s2m::ui_write_lambda(true); // IMPORTANT: this must be a blocking task, so that we can react to its results (e.g. report errors)
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
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         delete task;
         //
         if (error.text) {
            if (error.arg)
               luaL_argcheck(L, false, error.arg, error.text);
            cobb::lua::error(L, error.text, error.bad);
         }
         return 0;
      }
   }
   namespace _getters {
      int enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         bool result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::isEnabled);
         lua_pushboolean(L, result);
         return 1;
      }
      int layout_margins(lua_State* L) {
         lua_settop(L, 1);
         //
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         if (!_can_have_layout(*self.widget))
            return 0;
         QMargins result;
         {
            task_reference widget = (wrapped_type*)self.widget;
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [widget, &result]() {
               auto* layout = widget->layout();
               if (layout)
                  result = layout->contentsMargins();
            };
            core::subsystems::coordinator::get().send_ui_read_task(*task);
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
      int max_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::maximumHeight);
         if (result >= QWIDGETSIZE_MAX) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      int min_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::minimumHeight);
         if (result <= 0) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      int max_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::maximumWidth);
         if (result >= QWIDGETSIZE_MAX) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      int min_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         int result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::minimumWidth);
         if (result <= 0) {
            lua_pushnil(L);
            return 1;
         }
         lua_pushinteger(L, result);
         return 1;
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::objectName);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::toolTip);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
      int whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.widget)
            return 0;
         QString result = api_helpers::get_widget_property((wrapped_type*)self.widget, &wrapped_type::whatsThis);
         lua_pushstring(L, result.toUtf8());
         return 1;
      }
   }
   namespace _setters {
      int enabled(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         if (!self.widget)
            return 0;
         auto value = lua_toboolean(L, 2);
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setEnabled, value);
         return 0;
      }
      int layout_margins(lua_State* L) {
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
         task_reference widget = self.widget;
         auto* task = new tasks::s2m::ui_write_lambda(false);
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
         core::subsystems::coordinator::get().send_ui_write_task(*task);
         return 0;
      }
      int max_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            luaL_argcheck(L, lua_isnoneornil(L, 2), 2, "integer or nil expected");
            value = QWIDGETSIZE_MAX;
         } else {
            luaL_argcheck(L, value > 0, 2, "the size cannot be negative or zero");
            if (value > QWIDGETSIZE_MAX)
               value = QWIDGETSIZE_MAX;
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setMaximumHeight, value);
         return 0;
      }
      int min_height(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            luaL_argcheck(L, lua_isnoneornil(L, 2), 2, "integer or nil expected");
            value = 0;
         } else {
            luaL_argcheck(L, value > 0, 2, "the size cannot be negative or zero");
            if (value > QWIDGETSIZE_MAX)
               value = QWIDGETSIZE_MAX;
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setMinimumHeight, value);
         return 0;
      }
      int max_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            luaL_argcheck(L, lua_isnoneornil(L, 2), 2, "integer or nil expected");
            value = QWIDGETSIZE_MAX;
         } else {
            luaL_argcheck(L, value > 0, 2, "the size cannot be negative or zero");
            if (value > QWIDGETSIZE_MAX)
               value = QWIDGETSIZE_MAX;
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setMaximumWidth, value);
         return 0;
      }
      int min_width(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         int isnum;
         int value = lua_tointegerx(L, 2, &isnum);
         if (!isnum) {
            luaL_argcheck(L, lua_isnoneornil(L, 2), 2, "integer or nil expected");
            value = 0;
         } else {
            luaL_argcheck(L, value > 0, 2, "the size cannot be negative or zero");
            if (value > QWIDGETSIZE_MAX)
               value = QWIDGETSIZE_MAX;
         }
         if (!self.widget)
            return 0;
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setMinimumWidth, value);
         return 0;
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setObjectName, value);
         return 0;
      }
      int tooltip(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "tooltip text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setToolTip, value);
         return 0;
      }
      int whats_this(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "text (string) expected");
         if (!self.widget)
            return 0;
         auto value = QString::fromUtf8(lua_tostring(L, 2));
         api_helpers::set_widget_property((wrapped_type*)self.widget, &wrapped_type::setWhatsThis, value);
         return 0;
      }
   }

   namespace _singleton_functions {
      int new_(lua_State* L) {
         if (lua_gettop(L) > 0)
            cobb::lua::error(L, "the ui.widget.new function should not be called with a colon or passed any arguments");
         //
         core::subsystems::permissions::verify_ui_permissions();
         //
         wrapped_type* created = nullptr;
         auto*         task    = new tasks::s2m::ui_write_lambda(true);
         task->handler = [&created]() {
            created = new wrapped_type;
            DovahKitScriptVMCore::get().set_up_new_scripted_widget(created);
         };
         core::subsystems::coordinator::get().send_ui_write_task(*task);
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
      { "add_child",             &_methods::add_child },
      { "add_spacer",            &_methods::add_spacer },
      { "can_have_layout",       &_methods::can_have_layout },
      { "get_layout_stretch_at", &_methods::get_layout_stretch_at },
      { "on",                    &_methods::on },
      { "remove_child",          &_methods::remove_child },
      { "remove_event_listener", &_methods::remove_event_listener },
      { "set_layout",            &_methods::set_layout },
      { "set_layout_stretch_at", &_methods::set_layout_stretch_at },
      //
      // NOTE: When adding more member functions related  to the UI event system, be sure to 
      //       also amend any Lua-side classes that take events but don't derive from widget 
      //       e.g. radio_group.
      //
   };
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "enabled",        &_getters::enabled },
      { "layout_margins", &_getters::layout_margins },
      { "max_height",     &_getters::max_height },
      { "min_height",     &_getters::min_height },
      { "max_width",      &_getters::max_width },
      { "min_width",      &_getters::min_width },
      { "name",           &_getters::name },
      { "tooltip",        &_getters::tooltip },
      { "whats_this",     &_getters::whats_this },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "enabled",        &_setters::enabled },
      { "layout_margins", &_setters::layout_margins },
      { "max_height",     &_setters::max_height },
      { "min_height",     &_setters::min_height },
      { "max_width",      &_setters::max_width },
      { "min_width",      &_setters::min_width },
      { "name",           &_setters::name },
      { "tooltip",        &_setters::tooltip },
      { "whats_this",     &_setters::whats_this },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
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