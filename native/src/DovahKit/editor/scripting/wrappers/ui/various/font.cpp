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
   using namespace editor_script;
   using cls = wrappers::ui::font;
   using wrapped_type = cls::wrapped_type;

   // Helper functions for accessing whatever font we want to pull from the wrapper.
   QFont _get_font(const wrapper& w) {
      switch (w.type) {
         using wt = wrapper_type;
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
   void _set_font(wrapper& w, QFont f) {
      switch (w.type) {
         using wt = wrapper_type;
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

   /*
   
   TEMPLATE EXPERIMENTS:

   We need something like this so that getters, setters, and QFont::pull (a function to take a 
   plain table and write its fields into a new QFont) can share code, instead of us having to 
   effectively write two "set" functions per property.

   */



   template<typename lua_type> lua_type basic_pull_from_lua(lua_State* L) {
      lua_type raw = lua_type();
      if constexpr (std::is_same_v<lua_type, int>) {
         int isnum;
         raw = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
      } else if constexpr (std::is_same_v<lua_type, const char*>) {
         raw = lua_tostring(L, 2);
      } else if constexpr (std::is_same_v<lua_type, float> || std::is_same_v<lua_type, double>) {
         raw = lua_tonumber(L, 2);
      } else if constexpr (std::is_same_v<lua_type, bool>) {
         raw = lua_toboolean(L, 2);
      } else {
         // Lambda ugliness allows us to make static asserts conditional on their containing if-constexprs.
         []<bool flag = false>(){ static_assert(flag, "Unhandled Lua type!"); };
      }
      return raw;
   }
   template<typename lua_type> int basic_push_to_lua(lua_State* L, lua_type out) {
      if constexpr (std::is_same_v<lua_type, int>) {
         lua_pushinteger(L, out);
      } else if constexpr (std::is_same_v<lua_type, const char*>) {
         lua_pushstring(L, out);
      } else if constexpr (std::is_same_v<lua_type, float> || std::is_same_v<lua_type, double>) {
         lua_pushnumber(L, out);
      } else if constexpr (std::is_same_v<lua_type, bool>) {
         lua_pushboolean(L, out);
      } else {
         // Lambda ugliness allows us to make static asserts conditional on their containing if-constexprs.
         []<bool flag = false>(){ static_assert(flag, "Unhandled Lua type!"); };
      }
      return 1;
   }


   template<typename in_t, typename out_t = in_t> out_t no_op_c_to_lua(in_t f) {
      return f;
   }
   template<typename in_t, typename out_t = in_t> out_t no_op_lua_to_c(lua_State* L, in_t f) {
      return f;
   }

   // Defines a Lua getter for a font property. The first template parameter should be the 
   // QFont member function that retrieves the value. The optional second template parameter 
   // will be a function which takes that value and transforms it as necessary for use within 
   // Lua, e.g. switching zero-indexed integers to one-indexed or converting integer enums to 
   // string values.
   template<
      auto(QFont::* c_get)() const,
      auto adapt = no_op_c_to_lua<cobb::return_type_of<c_get>>
   > luastackchange_t font_getter(lua_State* L) {
      using c_type   = decltype((std::declval<QFont>().*c_get)());
      using lua_type = decltype((adapt)(c_type()));
      //
      auto&  self  = get_wrapper_for_thiscall<cls>(L);
      c_type value = c_type();
      {
         auto* task     = new tasks::s2m::ui_read_lambda();
         task->handler  = [&self, &value]() {
            auto font = _get_font(self);
            value = (font.*c_get)();
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
      }
      //
      lua_type out = (adapt)(value);
      return basic_push_to_lua(L, out);
   };

   // Defines a Lua setter for a font property; prefer the shorthand `font_setter` templates 
   // below. As with the getter, you specify a QFont member function to set the value, and 
   // an optional "adapt" function which transforms input from Lua into a format and type 
   // suitable for use with the QFont member function.
   template<
      typename c_type,
      typename lua_type,
      void(QFont::*c_set)(c_type),
      auto adapt = no_op_lua_to_c<lua_type, c_type>
   > luastackchange_t font_setter_impl(lua_State* L) {
      auto& self = get_wrapper_for_thiscall<cls>(L);
      //
      lua_type raw = basic_pull_from_lua<lua_type>(L);
      c_type value = adapt(L, raw);
      {
         auto* task = new tasks::s2m::ui_read_lambda();
         task->handler = [&self, &value]() {
            auto font = _get_font(self);
            (font.*c_set)(value);
            _set_font(self, font);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
      }
      return 0;
   }

   // Shorthand template for defining font setters.
   //
   // With "auto" template parameters, we can't meaningfully use them until we're actually in 
   // the function body. That means no "requires" criteria, and no ability to give (adapt) a 
   // default that depends on the traits of (setter).
   template<auto setter> luastackchange_t font_setter(lua_State* L) {
      static_assert(!cobb::is_const_function<setter>);
      using c_type = cobb::type_of_nth_argument<setter, 0>;
      return font_setter_impl<c_type, c_type, setter>(L);
   }
   template<auto setter, auto adapt> luastackchange_t font_setter(lua_State* L) {
      static_assert(std::is_same_v<lua_State*, cobb::type_of_nth_argument<adapt, 0>>, "The `adapt` function should take a Lua state as its first argument, so that it can throw errors upon receiving a bad value.");
      static_assert(!cobb::is_const_function<setter>);
      using c_type   = cobb::return_type_of<adapt>;
      using lua_type = cobb::type_of_nth_argument<adapt, 1>;
      static_assert(std::is_same_v<c_type, cobb::type_of_nth_argument<setter, 0>>);
      return font_setter_impl<c_type, lua_type, setter, adapt>(L);
   }

   template<auto setter> void pull_field_from_table(QFont& font, const char* name, lua_State* L, int table_pos) {
      static_assert(!cobb::is_const_function<setter>);
      using c_type = cobb::type_of_nth_argument<setter, 0>;
      //
      lua_getfield(L, table_pos, name);
      if (lua_isnoneornil(L, -1)) {
         lua_pop(L, 1);
         return;
      }
      //
      c_type raw = basic_pull_from_lua<c_type>(L);
      (font.*setter)(raw);
   }
   template<auto setter, auto adapt> void pull_field_from_table(QFont& font, const char* name, lua_State* L, int table_pos) {
      static_assert(std::is_same_v<lua_State*, cobb::type_of_nth_argument<adapt, 0>>, "The `adapt` function should take a Lua state as its first argument, so that it can throw errors upon receiving a bad value.");
      static_assert(!cobb::is_const_function<setter>);
      using c_type   = cobb::return_type_of<adapt>;
      using lua_type = cobb::type_of_nth_argument<adapt, 1>;
      //
      lua_getfield(L, table_pos, name);
      if (lua_isnoneornil(L, -1)) {
         lua_pop(L, 1);
         return;
      }
      //
      lua_type raw = basic_pull_from_lua<lua_type>(L);
      c_type value = adapt(L, raw); // TODO: pcall this
      (font.*setter)(value);
   }

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

namespace editor_script::wrappers::ui {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = {
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = {
      { "bold",           &_getters::bold },           // boolean; checks if the weight is bold
      { "capitalization", &_getters::capitalization }, // string enum
      { "italics",        &_getters::italics },        // boolean indicating whether the text is italicized
      { "letter_spacing", &_getters::letter_spacing }, // letter spacing, as a signed integer
      { "size",           &_getters::size },           // font size, e.g. "12px" or "12pt"
      { "weight",         &_getters::weight },         // font weight as a weighting scale from 1 to 100
      { "width",          &_getters::width },          // font-stretch as a percentage of normal font weight, e.g. 200% for twice as wide, or nil for "don't care"
   };
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = {
      { "bold",           &_setters::bold },
      { "capitalization", &_setters::capitalization },
      { "italics",        &_setters::italics },
      { "letter_spacing", &_setters::letter_spacing },
      { "size",           &_setters::size },
      { "weight",         &_setters::weight },
      { "width",          &_setters::width },
   };

   /*static*/ void cls::setup(lua_State* L) {
      int pos = lua_gettop(L);
      //
      // Create singleton:
      //
      lua_createtable(L, 0, 2);
      lua_pushcfunction(L, &_singleton_functions::is);
      lua_setfield     (L, -2, "is");
      //
      assert(lua_gettop(L) == pos + 1);
      lua_setfield(L, pos, cls::global_name);
   }
}