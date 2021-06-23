#include "font.h"
#include "../../../systems/editor_script_inner_core.h"
#include "../../../systems/lua_managed_resources.h"
#include "../../../systems/messaging.h"
#include "../../../systems/permissions.h"
#include "../../../systems/userdata.h"

#include "../../../wrapper_util.h"
#include "../../../collections.h"

#include "../../../ui/util/lua_item_model.h"

#include "../../../cross_thread_tasks/s2m/lambda.h"

#include "../../../../../helpers/lua/qt_variant.h"
#include "../../../../../helpers/function_traits.h"
#include "../../../../../helpers/strings.h"

#include "../../../widgets/objects/CanvasWidgetLayerDataLuaText.h"

#include <type_traits>

namespace {
   static constexpr int FONT_BOLD_WEIGHT_THRESHOLD = QFont::Bold;
}

namespace {
   QFont _get_font(const editor_script::wrapper& w) {
      switch (w.type) {
         using wt = editor_script::wrapper_type;
         case wt::ui: // widget
            if (auto* u = w.widget) {
               assert(w.parts[0].signature == wrapper_part_types::ui_font_data);
               return u->font();
            }
            break;
         case wt::ui_canvas_layer_data:
            if (auto* d = w.canvas_layer_data) {
               if (auto* tld = qobject_cast<CanvasWidgetLayerDataLuaText*>(d)) {
                  assert(w.parts[0].signature == wrapper_part_types::ui_font_data);
                  return tld->font;
               }
            }
            break;
         case wt::ui_model_item:
            if (auto* o = w.model_observer) {
               if (auto* i = o->item()) {
                  assert(w.parts[0].signature == wrapper_part_types::ui_font_role);
                  auto data = i->data(Qt::FontRole);
                  if (data.isValid() && data.type() == QMetaType::QFont)
                     return data.value<QFont>();
               }
            }
            break;
      }
      return QFont();
   }
   void _set_font(editor_script::wrapper& w, QFont f) {
      switch (w.type) {
         using wt = editor_script::wrapper_type;
         case wt::ui: // widget
            if (auto* u = w.widget) {
               u->setFont(f);
            }
            return;
         case wt::ui_canvas_layer_data:
            if (auto* d = w.canvas_layer_data) {
               if (auto* tld = qobject_cast<CanvasWidgetLayerDataLuaText*>(d)) {
                  tld->font = f;
               }
            }
            return;
         case wt::ui_model_item:
            if (auto* o = w.model_observer) {
               if (auto* i = o->item()) {
                  if (f == QFont()) {
                     //
                     // If it's a default font, yeet it.
                     //
                     i->setData(QVariant(), Qt::FontRole);
                  } else {
                     i->setData(f, Qt::FontRole);
                  }
               }
            }
            return;
      }
   }
}

namespace editor_script::impl::font_properties {
   namespace {
      int __pcall_pull_helper(lua_State* L) {
         auto* hnd    = (handler*) lua_touserdata(L, lua_upvalueindex(1));
         auto* result = (QVariant*) lua_touserdata(L, lua_upvalueindex(2));
         int top = lua_gettop(L);
         assert(top >= 1);
         //
         *result = (hnd->pull)(L, 1);
         return 0;
      }
      QVariant _pcall_pull(lua_State* L, int stack_pos, const handler& hnd) {
         QVariant result;
         //
         stack_pos = lua_absindex(L, stack_pos);
         lua_pushlightuserdata(L, (void*)&hnd);   // upvalue 1
         lua_pushlightuserdata(L, (void*)&result); // upvalue 2
         lua_pushcclosure(L, &__pcall_pull_helper, 2);
         lua_pushvalue(L, stack_pos);
         if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            lua_warning(L, "invalid value for property `", 1);
            lua_warning(L, hnd.name, 1);
            if (lua_isstring(L, -1)) {
               lua_warning(L, "`: ", 1);
               lua_warning(L, lua_tostring(L, -1), 0);
            } else {
               lua_warning(L, "`", 0);
            }
            result = QVariant();
         }
         //
         return result;
      }
   }
   namespace {
      extern int _getter(lua_State* L) {
         // Upvalue 1: string:         class metatable key (used to type-check self and get a valid wrapper-object)
         // Upvalue 2: light userdata: the handler set
         // Upvalue 3: light userdata: the handler name
         assert(lua_isstring(L, lua_upvalueindex(1)));
         assert(lua_islightuserdata(L, lua_upvalueindex(2)));
         assert(lua_isstring(L, lua_upvalueindex(3)));
         auto* class_metatable_key = lua_tostring(L, lua_upvalueindex(1));
         auto* class_handler_set   = (handler_set*) lua_touserdata(L, lua_upvalueindex(2));
         auto* property_name       = lua_tostring(L, lua_upvalueindex(3));
         assert(class_handler_set);
         assert(property_name && property_name[0]);
         //
         auto* wrap = (wrapper*) editor_script::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            return luaL_error(L, "function called with bad self (expected %s)", class_metatable_key);
         auto* hnd = class_handler_set->lookup(property_name);
         if (!hnd)
            return luaL_error(L, "property `%1` is not available here", property_name);
         //
         QVariant value;
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [hnd, &wrap, &value]() {
               auto font = _get_font(*wrap);
               value = (hnd->get)(font);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         return (hnd->push)(L, value);
      }
      extern int _setter(lua_State* L) {
         // Upvalue 1: string:         class metatable key (used to type-check self and get a valid wrapper-object)
         // Upvalue 2: light userdata: the handler set
         // Upvalue 3: light userdata: the handler name
         assert(lua_isstring(L, lua_upvalueindex(1)));
         assert(lua_islightuserdata(L, lua_upvalueindex(2)));
         assert(lua_isstring(L, lua_upvalueindex(3)));
         auto* class_metatable_key = lua_tostring (L, lua_upvalueindex(1));
         auto* class_handler_set   = (handler_set*) lua_touserdata(L, lua_upvalueindex(2));
         auto* property_name       = lua_tostring (L, lua_upvalueindex(3));
         assert(class_handler_set);
         assert(property_name && property_name[0]);
         //
         auto* wrap = (wrapper*)editor_script::cast_to_class(L, 1, class_metatable_key);
         if (wrap == nullptr)
            return luaL_error(L, "function called with bad self (expected %s)", class_metatable_key);
         const auto* hnd = class_handler_set->lookup(property_name);
         if (!hnd)
            return luaL_error(L, "property `%1` is not available here", property_name);
         //
         if (hnd->reset_if_nil && lua_isnoneornil(L, 2)) {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [hnd, &wrap, class_handler_set]() {
               QFont original = _get_font(*wrap);
               QFont modified;
               for (handler& other : *class_handler_set) {
                  if (&other == hnd)
                     continue;
                  if (!other.use_in_reset)
                     continue;
                  auto value = (other.get)(original);
                  if (value.isValid())
                     (other.set)(modified, value);
               }
               _set_font(*wrap, modified);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
            //
            return 0;
         }
         auto value = (hnd->pull)(L, 2);
         {
            auto* task = new tasks::s2m::ui_read_lambda();
            task->handler = [hnd, &wrap, &value]() {
               auto font = _get_font(*wrap);
               (hnd->set)(font, value);
               _set_font(*wrap, font);
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         //
         return 0;
      }
   }

   void handler_set::extend_lua_class(lua_State* L, const char* class_metatable_key, int getter_list_stack_pos, int setter_list_stack_pos) const noexcept {
      getter_list_stack_pos = lua_absindex(L, getter_list_stack_pos);
      setter_list_stack_pos = lua_absindex(L, setter_list_stack_pos);
      lua_checkstack(L, 5);
      for (auto& hnd : *this) {
         {  // Getter
            lua_pushstring       (L, class_metatable_key);
            lua_pushlightuserdata(L, (void*)this);
            lua_pushstring       (L, hnd.name);
            lua_pushcclosure(L, &_getter, 3);
         }
         lua_setfield(L, getter_list_stack_pos, hnd.name);
         {
            lua_pushstring       (L, class_metatable_key);
            lua_pushlightuserdata(L, (void*)this);
            lua_pushstring       (L, hnd.name);
            lua_pushcclosure(L, &_setter, 3);
         }
         lua_setfield(L, setter_list_stack_pos, hnd.name);
      }
   }
   QFont handler_set::table_to_struct(lua_State* L, int table_pos) const noexcept {
      table_pos = lua_absindex(L, table_pos);
      int top = lua_gettop(L);
      //
      QFont out;
      for(auto& hnd : *this) {
         auto t = lua_getfield(L, table_pos, hnd.name);
         if (t == LUA_TNONE || t == LUA_TNIL) {
            lua_settop(L, top);
            continue;
         }
         auto v = _pcall_pull(L, top + 1, hnd);
         if (v.isValid())
            (hnd.set)(out, v);
         lua_settop(L, top);
      }
      return out;
   }
}

namespace {
   using namespace editor_script;
   using cls = wrappers::ui::font;
   using wrapped_type = cls::wrapped_type;


   /*
   
   TEMPLATE EXPERIMENTS:

   We need something like this so that getters, setters, and QFont::pull (a function to take a 
   plain table and write its fields into a new QFont) can share code, instead of us having to 
   effectively write two "set" functions per property.

   */


   const char* capitalization_to_lua(QFont::Capitalization c) {
      switch (c) {
         using C = decltype(c);
         case C::MixedCase:
            return "normal";
         case C::AllUppercase:
            return "uppercase"; // HELLO, WORLD!
         case C::AllLowercase:
            return "lowercase"; // hello, world!
         case C::SmallCaps:
            return "small caps"; // "HELLO, WORLD!" but tiny
         case C::Capitalize:
            return "capitalize"; // Hello, World!
      }
      return "unknown";
   }
   QFont::Capitalization capitalization_from_lua(lua_State* L, const char* c) {
      assert(c);
      //
      using C = QFont::Capitalization;
      std::array list = {
         std::pair{ C::MixedCase,    "normal" },
         std::pair{ C::AllUppercase, "uppercase" },
         std::pair{ C::AllLowercase, "lowercase" },
         std::pair{ C::SmallCaps,    "small caps" },
         std::pair{ C::Capitalize,   "capitalize" }
      };
      for (auto& pair : list) {
         if (_stricmp(c, pair.second))
            return pair.first;
      }
      luaL_error(L, "unrecognized capitalization value: \"%s\"", c);
      return C::MixedCase;
   }












   /**/

   namespace _methods {
   }
   namespace _getters {
      luastackchange_t bold(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   value = QFont::Weight::Normal;
         {
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [&self, &value]() {
               value = _get_font(self).weight();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushboolean(L, value >= QFont::Medium);
         return 1;
      }
      luastackchange_t capitalization(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         QFont::Capitalization value = QFont::Capitalization::MixedCase;
         {
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [&self, &value]() {
               value = _get_font(self).capitalization();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         switch (value) {
            using C = QFont::Capitalization;
            case C::MixedCase:
               lua_pushstring(L, "normal"); // Hello, world!
               return 1;
            case C::AllUppercase:
               lua_pushstring(L, "uppercase"); // HELLO, WORLD!
               return 1;
            case C::AllLowercase:
               lua_pushstring(L, "lowercase"); // hello, world!
               return 1;
            case C::SmallCaps:
               lua_pushstring(L, "small caps"); // "HELLO, WORLD!" but tiny
               return 1;
            case C::Capitalize:
               lua_pushstring(L, "capitalize"); // Hello, World!
               return 1;
         }
         return 0;
      }
      luastackchange_t italics(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         QFont::Style style = QFont::Style::StyleNormal;
         {
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [&self, &style]() {
               style = _get_font(self).style();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         switch (style) {
            case QFont::StyleNormal:
               lua_pushboolean(L, false);
               return 1;
            case QFont::StyleItalic:
               lua_pushboolean(L, true);
               return 1;
            case QFont::StyleOblique:
               lua_pushboolean(L, true);
               return 1;
         }
         return 0;
      }
      luastackchange_t letter_spacing(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         qreal value = 0;
         {
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [&self, &value]() {
               auto font = _get_font(self);
               if (font.letterSpacingType() == QFont::SpacingType::AbsoluteSpacing)
                  value = _get_font(self).letterSpacing();
               else
                  value = 0;
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         lua_pushnumber(L, value);
         return 1;
      }
      luastackchange_t size(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   value = 0;
         bool  pixel = false;
         {
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [&self, &value, &pixel]() {
               auto font = _get_font(self);
               value = font.pixelSize();
               pixel = value >= 0;
               if (!pixel)
                  value = font.pointSize();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (pixel) {
            lua_pushfstring(L, "%dpx", value);
            return 1;
         } else {
            lua_pushfstring(L, "%dpt", value);
            return 1;
         }
         return 0;
      }
      luastackchange_t weight(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int   value = QFont::Weight::Normal;
         {
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [&self, &value]() {
               value = _get_font(self).weight();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         ++value; // [0, 99] -> [1, 100]
         lua_pushinteger(L, value);
         return 1;
      }
      luastackchange_t width(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         int value = 100;
         {
            auto* task     = new tasks::s2m::ui_read_lambda();
            task->handler  = [&self, &value]() {
               value = _get_font(self).stretch();
            };
            DovahKitScriptVMUITaskConduit::get().send_message(*task);
            delete task;
         }
         if (value == 0)
            return 0;
         lua_pushinteger(L, value);
         return 1;
      }
   }
   namespace _setters {

   }

   namespace _singleton_functions {
      luastackchange_t is(lua_State* L) {
         auto* wrapper = wrapper_from_stack<cls>(L, 1);
         lua_pushboolean(L, wrapper != nullptr);
         return 1;
      }
   }
}

namespace {
   template<typename T> int simple_push(lua_State* L, const QVariant& v) {
      if constexpr (std::is_same_v<T, bool>) {
         lua_pushboolean(L, v.toBool());
         return 1;
      }
      if constexpr (std::is_same_v<T, int>) {
         lua_pushinteger(L, v.toInt());
      } else if constexpr (std::is_same_v<T, const char*>) {
         lua_pushstring(L, v.toString().toUtf8());
      } else if constexpr (std::is_same_v<T, float>) {
         lua_pushnumber(L, v.toFloat());
      } else if constexpr (std::is_same_v<lua_type, double>) {
         lua_pushnumber(L, v.toDouble());
      } else if constexpr (std::is_same_v<T, bool>) {
         lua_pushboolean(L, v.toBool());
      } else {
         // Lambda ugliness allows us to make static asserts conditional on their containing if-constexprs.
         []<bool flag = false>(){ static_assert(flag, "Unhandled Lua type!"); };
      }
      return 1;
   }
   template<typename T, bool allow_nil = false> QVariant simple_pull(lua_State* L, int stack_pos) {
      if constexpr (allow_nil) {
         if (lua_isnoneornil(L, stack_pos))
            return QVariant();
      }
      //
      if constexpr (std::is_same_v<T, int>) {
         int isnum;
         int value = lua_tointegerx(L, stack_pos, &isnum);
         if (!isnum) {
            if constexpr (allow_nil)
               luaL_error(L, "integer or nil expected");
            else
               luaL_error(L, "integer expected");
         }
         return QVariant::fromValue<int>(value);
      } else if constexpr (std::is_same_v<T, const char*>) {
         if (!lua_isstring(L, stack_pos)) {
            if constexpr (allow_nil)
               luaL_error(L, "string or nil expected");
            else
               luaL_error(L, "string expected");
         }
         auto* value = lua_tostring(L, stack_pos);
         return QVariant::fromValue<QString>(QString::fromUtf8(value));
      } else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
         if (!lua_isnumber(L, stack_pos)) {
            if constexpr (allow_nil)
               luaL_error(L, "number or nil expected");
            else
               luaL_error(L, "number expected");
         }
         if constexpr (std::is_same_v<T, float>)
            return QVariant::fromValue<float>(lua_tonumber(L, stack_pos));
         QVariant::fromValue<double>(lua_tonumber(L, stack_pos));
      } else if constexpr (std::is_same_v<T, bool>) {
         if (!lua_isboolean(L, stack_pos)) {
            if constexpr (allow_nil)
               luaL_error(L, "boolean or nil expected");
            else
               luaL_error(L, "boolean expected");
         }
         return QVariant::fromValue<bool>(lua_toboolean(L, stack_pos));
      } else {
         // Lambda ugliness allows us to make static asserts conditional on their containing if-constexprs.
         []<bool flag = false>(){ static_assert(flag, "Unhandled Lua type!"); };
      }
      return QVariant();
   }

   template<auto func> int verbatim_push(lua_State* L, const QVariant& v) {
      static_assert(std::is_same_v<cobb::member_function_context_type<func>, QFont>, "The wrapped function must be a getter on QFont.");
      using T = cobb::return_type_of<func>;
      //
      return simple_push<T>(L, v);
   }
   template<auto func, bool allow_nil = false> QVariant verbatim_pull(lua_State* L, int stack_pos) {
      static_assert(std::is_same_v<cobb::member_function_context_type<func>, QFont>, "The wrapped function must be a setter on QFont.");
      using T = cobb::type_of_nth_argument<func, 0>;
      //
      return simple_pull<T, allow_nil>(L, stack_pos);
   }
   
   template<auto func, typename T> QVariant qvariant_get(const T& obj) {
      using out_t = cobb::return_type_of<func>;
      static_assert(std::is_base_of_v<cobb::function_traits<func>::context_type, T>, "The wrapped function must be a getter on templated type T.");
      static_assert(!cobb::is_const_function<func>, "The wrapped function must be a getter and therefore must be const.");
      static_assert(!std::is_same_v<out_t, void>, "The wrapped function must be a getter and therefore must return a value.");
      //
      out_t value = (obj.*func)();
      return QVariant::fromValue<out_t>(value);
   }
   template<auto func, typename T> void qvariant_set(T& obj, const QVariant& value) {
      using in_t = cobb::type_of_nth_argument<func, 0>;
      static_assert(std::is_base_of_v<cobb::function_traits<func>::context_type, T>, "The wrapped function must be a setter on templated type T.");
      static_assert(!cobb::is_const_function<func>, "The wrapped function must be a setter and therefore cannot be const.");
      //
      (obj.*func)(value.value<in_t>());
   }
}

namespace {
   namespace _fields {
      namespace bold {
         QVariant get(const QFont& font) {
            return QVariant::fromValue<bool>(font.weight() >= QFont::Medium);
         }
         void set(QFont& font, const QVariant& value) {
            bool after = value.toBool();
            bool prior = font.weight() >= QFont::Medium;
            if (after == prior)
               return;
            if (after)
               font.setWeight(QFont::Bold);
            else
               font.setWeight(QFont::Normal);
         }
         int push(lua_State* L, const QVariant& value) {
            lua_pushboolean(L, value.toBool());
            return 1;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isboolean(L, stack_pos))
               luaL_error(L, "boolean expected");
            return QVariant::fromValue<bool>(lua_toboolean(L, stack_pos));
         }
      }
      namespace capitalization {
         using C = QFont::Capitalization;
         static constexpr const std::array list = {
            std::pair{ C::MixedCase,    "normal" },
            std::pair{ C::AllUppercase, "uppercase" },
            std::pair{ C::AllLowercase, "lowercase" },
            std::pair{ C::SmallCaps,    "small caps" },
            std::pair{ C::Capitalize,   "capitalize" }
         };
         //
         int push(lua_State* L, const QVariant& value) {
            C v = (C)value.value<int>();
            for (auto& pair : list) {
               if (pair.first == v) {
                  lua_pushstring(L, pair.second);
                  return 1;
               }
            }
            return 0;
         }
         QVariant pull(lua_State* L, int stack_pos) {
            if (!lua_isstring(L, stack_pos))
               luaL_error(L, "string expected");
            auto* v = lua_tostring(L, stack_pos);
            for (auto& pair : list) {
               if (_stricmp(v, pair.second) == 0) {
                  return QVariant::fromValue<int>(pair.first);
               }
            }
            std::string error = "unrecognized value; valid values are: ";
            size_t size = list.size();
            for (size_t i = 0; i < size; ++i) {
               if (i)
                  error += ", ";
               error += '"';
               error += list[i].second;
               error += '"';
            }
            luaL_error(L, error.c_str());
            __assume(0);
         }
      }
   }
}

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      //
      // Fields that are handled as "font property handlers" should go in the list of those below, not here.
      //
      static_assert(false, "Finish converting all of these into FPHs.");
      { "bold",           &_getters::bold },           // boolean; checks if the weight is bold
      { "capitalization", &_getters::capitalization }, // string enum
      { "italics",        &_getters::italics },        // boolean indicating whether the text is italicized
      { "letter_spacing", &_getters::letter_spacing }, // letter spacing, as a signed integer
      { "size",           &_getters::size },           // font size, e.g. "12px" or "12pt"
      { "weight",         &_getters::weight },         // font weight as a weighting scale from 1 to 100
      { "width",          &_getters::width },          // font-stretch as a percentage of normal font weight, e.g. 200% for twice as wide, or nil for "don't care"
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      //
      // Fields that are handled as "font property handlers" should go in the list of those below, not here.
      //
   };

   using fph = editor_script::impl::font_properties::handler;
   /*static*/ const editor_script::impl::font_properties::handler_set cls::fph_handlers = {{
      fph{ 
         .name = "bold",
         .push = _fields::bold::push,
         .pull = _fields::bold::pull,
         .get  = _fields::bold::get,
         .set  = _fields::bold::set,
         .reset_if_nil = false,
         .use_in_reset = false,
      },
      fph{ 
         .name = "capitalization",
         .push = _fields::capitalization::push,
         .pull = _fields::capitalization::pull,
         .get  = qvariant_get<QFont::capitalization>,
         .set  = qvariant_set<QFont::setCapitalization>,
         .reset_if_nil = true
      },
      fph{ 
         .name = "letter_spacing",
         .push = verbatim_push<QFont::letterSpacing>,
         .pull = verbatim_pull<QFont::setLetterSpacing, true>,
         .get  = qvariant_get<QFont::letterSpacing>,
         .set  = qvariant_set<QFont::setLetterSpacing>,
         .reset_if_nil = true
      },
      fph{ 
         .name = "weight",
         .push = verbatim_push<QFont::weight>,
         .pull = verbatim_pull<QFont::setWeight, true>,
         .get  = qvariant_get<QFont::weight>,
         .set  = qvariant_set<QFont::setWeight>,
         .reset_if_nil = true
      },
   }};

   /*static*/ void cls::extra_class_setup(lua_State* L) noexcept {
      int index_class   = lua_absindex(L, -3);
      int index_getters = lua_absindex(L, -2);
      int index_setters = lua_absindex(L, -1);
      //
      cls::fph_handlers.extend_lua_class(L, cls::metatable_key, index_getters, index_setters);
   }
}