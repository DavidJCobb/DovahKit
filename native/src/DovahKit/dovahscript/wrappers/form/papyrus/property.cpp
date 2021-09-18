#include "property.h"
#include "../../../../helpers/lua/error.h"
#include "../../../../helpers/lua/warning.h"
#include "../../../../helpers/strings.h"
#include "../../../core/subsystems/permissions.h"
#include "../../../core/subsystems/userdata.h"
#include "../../../core/classes.h"
#include "../../../core/collections.h"
#include "../../../push_native_object.h"

#include "../../../../dovah/form_stub.h"
#include "../../../../dovah/forms/Form.h"
#include "../../../../dovah/forms/Quest.h"

#include "script.h"
#include "property/collection_array_values.h"
#include "../../../api_helpers/papyrus_property_values.h"

#include "../form.h"
#include "../quest/alias.h"

namespace {
   using namespace dovahscript;
   using wrapper_t = wrappers::papyrus_property;
}

namespace { // helpers
   using papyrus_property_type = dovah::loaded_forms::components::papyrus::property_type;
   std::array<std::pair<papyrus_property_type, const char*>, 10> papyrus_typenames = {{
      { papyrus_property_type::boolean,          "Bool" },
      { papyrus_property_type::float32,          "Float" },
      { papyrus_property_type::string,           "String" },
      { papyrus_property_type::integer,          "Int" },
      { papyrus_property_type::object,           "FormOrAlias" },
      { papyrus_property_type::array_of_boolean, "Bool[]" },
      { papyrus_property_type::array_of_float32, "Float[]" },
      { papyrus_property_type::array_of_string,  "String[]" },
      { papyrus_property_type::array_of_integer, "Int[]" },
      { papyrus_property_type::array_of_object,  "FormOrAlias[]" },
   }};
}

namespace {
   namespace _getters {
      int is_array(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr) {
            lua_pushnil(L);
            return 1;
         }
         //
         using pt = dovah::loaded_forms::components::papyrus::property_type;
         switch (prop->type) {
            case pt::boolean:
            case pt::float32:
            case pt::string:
            case pt::integer:
            case pt::object:
               lua_pushboolean(L, false);
               return 1;
               //
            case pt::array_of_boolean:
            case pt::array_of_float32:
            case pt::array_of_string:
            case pt::array_of_integer:
            case pt::array_of_object:
               lua_pushboolean(L, true);
               return 1;
         }
         //
         lua_pushnil(L);
         return 1;
      }
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            cobb::lua::error(L, "script property wrapper has no underlying object (deleted?)");
         //
         lua_pushstring(L, prop->name.c_str());
         return 1;
      }
      int type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            cobb::lua::error(L, "script property wrapper has no underlying object (deleted?)");
         //
         for (auto& p : papyrus_typenames) {
            if (p.first == prop->type) {
               lua_pushstring(L, p.second);
               return 1;
            }
         }
         lua_pushstring(L, "Invalid");
         return 1;
      }
      int value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            cobb::lua::error(L, "script property wrapper has no underlying object (deleted?)");
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
                        return push_native_object(quest);
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
                  out.append_part(dovahscript::wrapper_part_types::papyrus_property_array_value);
                  out.is_collection = true;
                  return core::subsystems::userdata::get().push(L, out, wrappers::collections::papyrus_property_array_value_list.registry_key);
               }
         }
         return 0;
      }
   }
   namespace _setters {
      int name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            cobb::lua::error(L, "script property wrapper has no underlying object (deleted?)");
         //
         auto* name   = lua_tolstring(L, 2, nullptr);
         auto* script = wrappers::papyrus_script::unwrap(self, false);
         assert(script && "How did we manage to access a Papyrus property if we didn't manage to access its containing Papyrus script-object?");
         for (auto& p : script->properties)
            if (&p != prop && _stricmp(p.name.c_str(), name) == 0)
               cobb::lua::error(L, "the containing script already has a property named `%s`", name);
         //
         self.before_edit();
         prop->name = name;
         self.after_edit();
         return 0;
      }
      int type(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            cobb::lua::error(L, "script property wrapper has no underlying object (deleted?)");
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "type (string) expected");
         //
         std::string type = cobb::trim(lua_tostring(L, 2));
         size_t      size = type.size();
         if (size < 0)
            cobb::lua::error(L, "`%s` is not something Lua can recognize as a Papyrus typename", type.c_str());
         //
         auto typeval = papyrus_property_type::integer;
         bool match   = false;
         for (auto& e : papyrus_typenames) {
            if (_stricmp(type.c_str(), e.second) == 0) {
               typeval = e.first;
               match = true;
               break;
            }
         }
         if (!match)
            cobb::lua::error(L, "`%s` is not something Lua can recognize as a Papyrus typename", type.c_str());
         if (prop->type == typeval)
            return 0;
         //
         self.before_edit();
         prop->set_type(*self.form, typeval);
         {  // Zombify any extant wrappers for the array, if its type has changed.
            wrapper zombie = self;
            zombie.append_part(dovahscript::wrapper_part_types::papyrus_property_array_value);
            zombie.is_collection = true;
            core::subsystems::userdata::get().destroy(zombie);
         }
         self.after_edit();
         return 0;
      }
      int value(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            cobb::lua::error(L, "script property wrapper has no underlying object (deleted?)");
         //
         int  isnum;
         auto vt = lua_type(L, 2);
         if (prop->is_array()) {
            cobb::lua::argcheck(L, vt == LUA_TUSERDATA || vt == LUA_TTABLE, 2, "attempted to set a Papyrus array property to the wrong type");
            auto* other = (wrapper*) classes::cast_to_class(L, 2, wrappers::collections::papyrus_property_array_value_list.registry_key);
            if (other) {
               if (self.is_equal(other)) // early-out on self-assignment
                  return 0;
               auto* source = wrappers::papyrus_property::unwrap(*other, false);
               cobb::lua::argcheck(L, source, 2, "the provided property-value wrapper has no underlying object (deleted?)");
               self.before_edit();
               auto& lf = *self.form;
               prop->clear(lf);
               prop->clone_from(*source, lf);
               self.after_edit();
               return 0;
            }
            //
            // No way to adequately vet the type of the passed-in value, so just treat it as an array.
            //
            lua_len(L, 2);
            int length = lua_tointegerx(L, -1, &isnum);
            lua_pop(L, 1);
            cobb::lua::argcheck(L, isnum, 2, "provided table or userdata did not return a valid result for its __len operator");
            for (int i = 1; i <= length; ++i) {
               lua_geti(L, 2, i);
               bool valid = dovahscript::api_helpers::papyrus::property_scalar_value_typecheck(L, -1, prop->type);
               lua_pop(L, 1);
               if (!valid)
                  cobb::lua::error(L, "provided array has a value of the wrong type at index %d", i);
            }
            //
            self.before_edit();
            prop->clear(*self.form);
            prop->values.resize(length);
            for (int i = 1; i <= length; ++i) {
               lua_geti(L, 2, i);
               dovahscript::api_helpers::papyrus::set_property_value(L, -1, self, *prop, i - 1);
               lua_pop(L, 1);
            }
            self.after_edit();
         } else {
            cobb::lua::argcheck(L, dovahscript::api_helpers::papyrus::property_scalar_value_typecheck(L, 2, prop->type), 2, "desired value is of the wrong type for this property");
            //
            self.before_edit();
            prop->clear(*self.form);
            prop->values.resize(1);
            dovahscript::api_helpers::papyrus::set_property_value(L, 2, self, *prop, 0);
            self.after_edit();
         }
         return 0;
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_getters = {
      { "is_array", &_getters::is_array },
      { "name",     &_getters::name },
      { "type",     &_getters::type },
      { "value",    &_getters::value },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_t::metatable_setters = {
      { "name",  &_setters::name },
      { "type",  &_setters::type },
      { "value", &_setters::value },
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

   /*static*/ void wrapper_t::extra_class_setup(lua_State* L) {
      define_collection_metatable(L, collections::papyrus_property_array_value_list);
   }
}