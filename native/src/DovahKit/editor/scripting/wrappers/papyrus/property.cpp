#include "property.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"

#include "../../../../dovah/forms/Form.h"
#include "root.h"

namespace {
   using namespace editor_script;
   using wrapper_t = wrappers::papyrus_property;
   //
   namespace _getters {
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
         //
         lua_pushstring(L, prop->name.c_str());
         return 1;
      }
      luastackchange_t value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
         //
         using pt = dovah::loaded_forms::components::papyrus::property_type;
         switch (prop->type) {
            case pt::boolean:
               lua_pushboolean(L, prop->values[0].boolean);
               return 1;
            case pt::float32:
               lua_pushnumber(L, prop->values[0].float32);
               return 1;
            case pt::string:
               lua_pushstring(L, prop->values[0].string.c_str());
               return 1;
            case pt::integer:
               lua_pushinteger(L, prop->values[0].integer);
               return 1;
            case pt::object:
               {
                  auto& value = prop->values[0].object;
                  auto* quest = value.form.get_form_stub();
                  //
                  wrapper out;
                  auto* mt = wrap_form(out, quest);
                  if (quest && value.aliasID != -1) {
                     assert(false && "returning quest aliases to Lua is not yet implemented!");
                     #if !_DEBUG
                        static_assert(false, "DO NOT FORGET to implement returning quest aliases to Lua, both from a quest and from script properties!");
                     #endif
                  }
                  return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
               }
            //
            // TODO: array values
            //
         }
         //
         lua_pushstring(L, prop->name.c_str());
         return 1;
      }
   }
   namespace _setters {
      luastackchange_t name(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
         //
         self.before_edit();
         prop->name = lua_tolstring(L, 2, nullptr);
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_getters = {
      { "name", &_getters::name },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_setters = {
      { "name", &_setters::name },
   };

   /*static*/ wrapper_t::wrapped_t* wrapper_t::unwrap(wrapper& w) {
      if (w.is_collection)
         return nullptr;
      if (w.parts[1].signature != cobb::eight_cc("PapyScri"))
         return nullptr;
      if (w.parts[2].signature != cobb::eight_cc("PapyProp"))
         return nullptr;
      auto* root = papyrus_root::unwrap(w);
      if (!root)
         return nullptr;
      auto& list = root->scripts;
      auto  i    = w.parts[1].index;
      if (i >= list.size())
         return nullptr;
      auto& scr = list[i];
      i = w.parts[2].index;
      if (i >= scr.properties.size())
         return nullptr;
      return &scr.properties[i];
   }
}