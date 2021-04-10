#include "property.h"
#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../editor_script_core.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

#include "../../../../dovah/forms/Form.h"
#include "script.h"

#include "../quest/alias.h"

namespace {
   using namespace editor_script;
   using wrapper_t = wrappers::papyrus_property;
}

#pragma region Collection: "array values"
namespace {
   namespace _collections::_array_values {
      wrapper& get_collection_wrapper(lua_State* L) {
         auto* self = (wrapper*)editor_script::cast_to_class(L, 1, wrapper_t::array_collection_key);
         if (self == nullptr) {
            luaL_error(L, "function called with bad self (expected %s)", wrapper_t::array_collection_key);
         }
         return *self;
      }

      luastackchange_t get_collection_length(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, false);
         if (!prop) {
            lua_pushinteger(L, 0);
            return 1;
         }
         using pt = dovah::loaded_forms::components::papyrus::property_type;
         switch (prop->type) {
            case pt::boolean:
            case pt::float32:
            case pt::string:
            case pt::integer:
            case pt::object:
               lua_pushinteger(L, 0);
               return 1;
            case pt::array_of_boolean:
            case pt::array_of_float32:
            case pt::array_of_string:
            case pt::array_of_integer:
            case pt::array_of_object:
               lua_pushinteger(L, prop->values.size());
               return 1;
         }
         lua_pushinteger(L, 0);
         return 1;
      }
      luastackchange_t lookup_item_by_index(lua_State* L) {
         auto& self = get_collection_wrapper(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, false);
         if (!prop)
            return 0;
         auto  i    = lua_tointeger(L, 2);
         auto& list = prop->values;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         //
         using pt = dovah::loaded_forms::components::papyrus::property_type;
         switch (prop->type) {
            case pt::boolean:
               lua_pushboolean(L, prop->values[i].boolean);
               return 1;
            case pt::float32:
               lua_pushnumber(L, prop->values[i].float32);
               return 1;
            case pt::string:
               lua_pushstring(L, prop->values[i].string.c_str());
               return 1;
            case pt::integer:
               lua_pushinteger(L, prop->values[i].integer);
               return 1;
            case pt::object:
               {
                  auto& value = prop->values[i].object;
                  auto* quest = value.form.get_form_stub();
                  //
                  if (quest) {
                     if (value.aliasID != -1) {
                        return wrappers::quest_alias::wrap(L, quest, value.aliasID);
                     } else {
                        wrapper out;
                        auto* mt = wrap_form(out, quest);
                        return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
                     }
                  }
                  return 0;
               }
         }
         return 0;
      }
   }
}
#pragma endregion

namespace {
   namespace _getters {
      luastackchange_t name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
         //
         lua_pushstring(L, prop->name.c_str());
         return 1;
      }
      luastackchange_t value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
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
                  if (quest) {
                     if (value.aliasID != (decltype(value.aliasID))dovah::loaded_forms::Alias::none_id) {
                        return wrappers::quest_alias::wrap(L, quest, value.aliasID);
                     } else {
                        wrapper out;
                        auto* mt = wrap_form(out, quest);
                        return DovahKitScriptVMUserdataInterface::get().push(L, out, mt);
                     }
                  }
                  return 0;
               }
            case pt::array_of_boolean:
            case pt::array_of_float32:
            case pt::array_of_string:
            case pt::array_of_integer:
            case pt::array_of_object:
               {
                  wrapper out = self;
                  out.append_part(editor_script::wrapper_part_types::papyrus_property_array_value);
                  out.is_collection = true;
                  return DovahKitScriptVMUserdataInterface::get().push(L, out, wrapper_t::array_collection_key);
               }
         }
         return 0;
      }
   }
   namespace _setters {
      luastackchange_t name(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
         //
         auto* name   = lua_tolstring(L, 2, nullptr);
         auto* script = wrappers::papyrus_script::unwrap(self, false);
         assert(script && "How did we manage to access a Papyrus property if we didn't manage to access its containing Papyrus script-object?");
         for (auto& p : script->properties)
            if (&p != prop && _stricmp(p.name.c_str(), name) == 0)
               luaL_error(L, "the containing script already has a property named \"%s\"", name);
         //
         self.before_edit();
         prop->name = name;
         self.after_edit();
         return 0;
      }
   }
}

namespace editor_script::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_getters = {
      { "name",  &_getters::name },
      { "value", &_getters::value },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_setters = {
      { "name", &_setters::name },
   };

   /*static*/ wrapper_t::wrapped_t* wrapper_t::unwrap(wrapper& w, bool must_be_end) {
      uint8_t next;
      return wrapper_t::unwrap(w, must_be_end, next);
   }
   /*static*/ wrapper_t::wrapped_t* wrapper_t::unwrap(wrapper& w, bool must_be_end, uint8_t& next_depth) {
      uint8_t next;
      auto*   script = papyrus_script::unwrap(w, false, next);
      next_depth = next;
      if (!script)
         return nullptr;
      if (w.parts[next].signature != wrapper_part_types::papyrus_property)
         return nullptr;
      if (w.is_collection_at_depth(next))
         return nullptr;
      auto i = w.parts[next].index;
      if (i >= script->properties.size())
         return nullptr;
      ++next_depth;
      if (must_be_end && w.depth != next_depth)
         return nullptr;
      return &script->properties[i];
   }

   /*static*/ void wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key           = wrapper_t::array_collection_key,
         .garbage_collection     = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::_array_values::get_collection_length,
         .lookup_item_by_index   = &_collections::_array_values::lookup_item_by_index,
         //.member_function_insert = &_collections::_array_values::member_function_insert, // TODO
         //.member_function_remove = &_collections::_array_values::member_function_remove, // TODO
         //.set_item               = &_collections::_array_values::set_item,               // TODO
      });
   }
}