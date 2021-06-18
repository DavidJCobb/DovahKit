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

#include "../../../widgets/objects/CanvasWidgetLayerDataLuaText.h"

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

   template<auto(*access)(QFont)> luastackchange_t font_property_getter(lua_State* L) {
      using out_t = decltype((access)(QFont()));
      //
      auto& self  = get_wrapper_for_thiscall<cls>(L);
      out_t value = out_t();
      {
         auto* task     = new tasks::s2m::ui_read_lambda();
         task->handler  = [&self, &value]() {
            auto font = _get_font(self);
            value = (access)(font);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         delete task;
      }
      if constexpr (std::is_same_v<out_t, int>) {
         lua_pushinteger(L, value);
      } else if constexpr (std::is_same_v<out_t, const char*>) {
         lua_pushstring(L, value);
      } else if constexpr (std::is_same_v<out_t, float> || std::is_same_v<arg_ctype, double>) {
         lua_pushnumber(L, value);
      } else if constexpr (std::is_same_v<out_t, bool>) {
         lua_pushboolean(L, value);
      } else {
         assert(false);
         return 0;
      }
      return 1;
   }
   template<typename arg_ctype, QFont(*modify)(QFont, arg_ctype)> luastackchange_t font_property_setter(lua_State* L) {
      auto&     self = get_wrapper_for_thiscall<cls>(L);
      arg_ctype value;
      if constexpr (std::is_same_v<arg_ctype, int>) {
         int isnum;
         value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
      } else if constexpr (std::is_same_v<arg_ctype, const char*>) {
         value = lua_tostring(L, 2);
         luaL_argcheck(L, value != nullptr, 2, "string expected");
      } else if constexpr (std::is_same_v<arg_ctype, float> || std::is_same_v<arg_ctype, double>) {
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         value = lua_tonumber(L, 2);
      } else if constexpr (std::is_same_v<arg_ctype, bool>) {
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         value = lua_toboolean(L, 2);
      } else {
         assert(false);
      }
      {
         auto* task = new tasks::s2m::lambda(false);
         task->handler = [&self, &value]() {
            auto font = _get_font(self);
            font = (modify)(font, value);
            _set_font(self, font);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
      }
      return 0;
   }
   template<typename arg_ctype, QFont(*modify)(QFont, arg_ctype), QFont(*reset)(QFont)> luastackchange_t font_property_setter(lua_State* L) {
      auto& self = get_wrapper_for_thiscall<cls>(L);
      if (lua_isnoneornil(L, 2)) {
         auto* task = new tasks::s2m::lambda(false);
         task->handler = [&self]() {
            auto font = _get_font(self);
            font = (reset)(font);
            _set_font(self, font);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      arg_ctype value;
      if constexpr (std::is_same_v<arg_ctype, int>) {
         int isnum;
         value = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "integer expected");
      } else if constexpr (std::is_same_v<arg_ctype, const char*>) {
         value = lua_tostring(L, 2);
         luaL_argcheck(L, value != nullptr, 2, "string expected");
      } else if constexpr (std::is_same_v<arg_ctype, float> || std::is_same_v<arg_ctype, double>) {
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "number expected");
         value = lua_tonumber(L, 2);
      } else if constexpr (std::is_same_v<arg_ctype, bool>) {
         luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
         value = lua_toboolean(L, 2);
      } else {
         assert(false);
      }
      {
         auto* task = new tasks::s2m::lambda(false);
         task->handler = [&self, &value]() {
            auto font = _get_font(self);
            font = (modify)(font, value);
            _set_font(self, font);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
      }
      return 0;
   }

   /*
   
   In theory, with the above functions, we can do:

      namespace _fields {
         namespace bold {
            bool  access(QFont);
            QFont modify(QFont, bool);
         };
      };

      const std::initializer_list<luaL_Reg> cls::metatable_getters = {
         { "bold", &font_property_getter<_fields::bold::access> },
      };
      const std::initializer_list<luaL_Reg> cls::metatable_setters = {
         { "bold", &font_property_setter<bool, _fields::bold::modify> },
      };

   It's not perfectly clean. Having to specify the "bare" value (bool) as a template parameter 
   for the setter is deeply unpleasant. Maybe we can have the (modify) template-parameter's 
   second argument type be (auto), and then use this?:
   https://stackoverflow.com/questions/28509273/get-types-of-c-function-parameters

   Some other defects with the system above is that there's no way to error-check any values 
   passed in from Lua. What we could do is have an additional template parameter for a function 
   which takes the input type and returns a const char* error string, with that parameter set 
   to a no-op that always returns nullptr for cases where no argument validation is needed. It 
   might almost be cleaner, however, to instead have the (modify) function take an out-argument 
   that can be used to signal error information, e.g.

      QFont modify(QFont, bool value, std::string& error);

   Lastly, requiring two separate templates depending on whether the setter should allow nil is 
   really ugly. Can we find a way to condense that into one, somehow?
   
   */

   namespace _fields {
      namespace bold {
         QFont set(QFont font, bool value) {
            auto weight = font.weight();
            if (value) {
               if (weight < FONT_BOLD_WEIGHT_THRESHOLD)
                  font.setWeight(QFont::Weight::Bold);
            } else {
               if (weight >= FONT_BOLD_WEIGHT_THRESHOLD)
                  font.setWeight(QFont::Weight::Normal);
            }
            return font;
         }
      }
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
      luastackchange_t data(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto& vm    = DovahKitScriptVMCore::get();
         if (lua_type(L, 2) == LUA_TTABLE) {
            luaL_error(L, "storing a table as a dropdown item's data member is not supported");
         }
         auto  value = vm.variant_from_lua(2);
         if (!value.isValid() && !lua_isnoneornil(L, 2)) {
            luaL_error(L, "the provided value cannot be stored as a dropdown item's data member");
         }
         if (!self.model_observer)
            return 0;
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         task->handler  = [observer, value]() {
            if (auto* item = observer->item())
               item->setData(value, Qt::UserRole);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t icon(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         if (!self.model_observer)
            return 0;
         //
         LuaManagedResourceHandle value; // must use a handle here, to avoid race conditions that stem from this being non-blocking (i.e. Lua var goes out of scope, gets closed or GC'd, before we send the resource to Qt)
         if (!lua_isnoneornil(L, 2)) {
            auto* arg = wrapper_from_stack<wrappers::resource::raster>(L, 2);
            luaL_argcheck(L, arg != nullptr, 2, "raster expected");
            value = arg->managed_resource;
         }
         //
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         task->handler  = [observer, value]() mutable {
            if (auto* item = observer->item()) {
               auto wrapped = QVariant::fromValue<LuaManagedResourceHandle>(value);
               item->setData(wrapped, Qt::DecorationRole);
            }
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
      luastackchange_t text(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "string expected");
         if (!self.model_observer)
            return 0;
         auto* observer = self.model_observer;
         auto* task     = new tasks::s2m::lambda(false);
         auto  value    = QString::fromUtf8(lua_tostring(L, 2));
         task->handler  = [observer, value]() {
            if (auto* item = observer->item())
               item->setData(value, Qt::DisplayRole);
         };
         DovahKitScriptVMUITaskConduit::get().send_message(*task);
         return 0;
      }
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