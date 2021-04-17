#include "property.h"
#include "../../systems/permissions.h"
#include "../../systems/userdata.h"

#include "../../classes.h"
#include "../../util.h"
#include "../../../../dovah/form_stub.h"
#include "../../wrapper_util.h"
#include "../../collections.h"

#include "../../../../dovah/forms/Form.h"
#include "script.h"

#include "../../../../helpers/strings.h"

#include "../form.h"
#include "../quest/alias.h"

namespace {
   using namespace editor_script;
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

   bool property_scalar_value_typecheck(lua_State* L, int stack_pos, papyrus_property_type pt) {
      pt = dovah::loaded_forms::components::papyrus::scalar_property_type_for(pt);
      switch (pt) {
         case papyrus_property_type::boolean:
            return lua_isboolean(L, stack_pos);
         case papyrus_property_type::float32:
            return lua_isnumber(L, stack_pos);
         case papyrus_property_type::integer:
            {
               int isnum;
               lua_tointegerx(L, stack_pos, &isnum);
               return isnum != 0;
            }
            break;
         case papyrus_property_type::object:
            {
               if (lua_isnoneornil(L, stack_pos))
                  return true;
               stack_pos = lua_absindex(L, stack_pos);
               if (auto* form = wrapper_from_stack<wrappers::form>(L, stack_pos))
                  return true;
               if (auto* alias = wrapper_from_stack<wrappers::quest_alias>(L, stack_pos))
                  return true;
               return false;
            }
            break;
         case papyrus_property_type::string:
            return lua_isstring(L, stack_pos);
      }
      return false;
   }

   void set_papyrus_property_value(lua_State* L, int stack_pos, wrapper& wrapper, wrapper_t::wrapped_t& prop, size_t index) {
      auto& lf = *wrapper.form;
      auto  st = prop.scalar_type();
      auto& v  = prop.values[index];
      prop.values[index].clear(lf);
      switch (st) {
         case papyrus_property_type::boolean:
            v.boolean = lua_toboolean(L, stack_pos);
            break;
         case papyrus_property_type::float32:
            v.float32 = lua_tonumber(L, stack_pos);
            break;
         case papyrus_property_type::integer:
            {
               int isnum;
               int i = lua_tointegerx(L, stack_pos, &isnum);
               assert(isnum); // we should already have checked this, above
               v.integer = i;
            }
            break;
         case papyrus_property_type::object:
            if (lua_isnoneornil(L, stack_pos))
               break;
            if (auto* form = wrapper_from_stack<wrappers::form>(L, stack_pos)) {
               v.object.form.set(lf, form->stub);
               break;
            }
            if (auto* alias_wrapper = wrapper_from_stack<wrappers::quest_alias>(L, stack_pos)) {
               if (auto* alias = wrappers::quest_alias::unwrap(*alias_wrapper)) {
                  v.object.form.set(lf, alias_wrapper->stub);
                  v.object.aliasID = alias->id;
               }
               break;
            }
            break;
         case papyrus_property_type::string:
            v.string = lua_tostring(L, stack_pos);
            break;
      }
   }
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
         auto& self = get_collection_wrapper(L);
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
            case pt::array_of_boolean:
               lua_pushboolean(L, prop->values[i].boolean);
               return 1;
            case pt::array_of_float32:
               lua_pushnumber(L, prop->values[i].float32);
               return 1;
            case pt::array_of_string:
               lua_pushstring(L, prop->values[i].string.c_str());
               return 1;
            case pt::array_of_integer:
               lua_pushinteger(L, prop->values[i].integer);
               return 1;
            case pt::array_of_object:
               {
                  auto& value = prop->values[i].object;
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
         }
         return 0;
      }
      luastackchange_t member_function_insert(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_collection_wrapper(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, false);
         //
         int  pos_value = 2;
         bool has_index = false;
         //
         if (lua_gettop(L) >= 3) {
            has_index = true;
            pos_value = 3;
            luaL_argcheck(L, lua_isinteger(L, 2), 2, "provided index is not an integer");
         }
         if (!property_scalar_value_typecheck(L, pos_value, prop->type))
            luaL_error(L, "desired value is of the wrong type for this property");
         //
         if (!prop)
            return 0;
         auto& list = prop->values;
         auto  size = list.size();
         int   i    = size + 1;
         if (has_index) {
            i = lua_tointeger(L, 2);
            if (i < 1)
               return luaL_error(L, "indices below 1, such as %d, are not allowed", i);
            --i;
         }
         if (i >= size) {
            if (i > size) {
               lua_warning(L, "index ", 1);
               const char* tostr = lua_tolstring(L, 2, nullptr);
               lua_warning(L, tostr, 1);
               lua_warning(L, " is out of bounds; nil elements will be created between the end of the list and the new element", 0);
            }
            list.resize(i + 1);
         } else {
            list.emplace(list.begin() + i);
         }
         self.before_edit();
         set_papyrus_property_value(L, pos_value, self, *prop, i);
         self.after_edit();
         return 0;
      }
      luastackchange_t member_function_remove(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_collection_wrapper(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, false);
         luaL_argcheck(L, lua_isnumber(L, 2), 2, "expected an integer index");
         int isnum;
         int i = lua_tointegerx(L, 2, &isnum);
         luaL_argcheck(L, isnum, 2, "expected an integer index");
         if (!prop)
            return 0;
         auto& list = prop->values;
         if (i > list.size() || i <= 0)
            return 0;
         --i;
         self.before_edit();
         list[i].clear(*self.form);
         list.erase(list.begin() + i);
         self.after_edit();
         return 0;
      }
      luastackchange_t set_item(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         constexpr auto index_self  = 1;
         constexpr auto index_key   = 2;
         constexpr auto index_value = 3;
         //
         int isnum;
         int i = lua_tointegerx(L, index_key, &isnum);
         if (!isnum)
            luaL_error(L, "cannot use string keys or non-integer keys");
         if (i < 1)
            return luaL_error(L, "indices below 1, such as %d, are not allowed", i);
         --i;
         //
         auto& self = get_collection_wrapper(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, false);
         if (!prop)
            return 0;
         //
         if (!property_scalar_value_typecheck(L, index_value, prop->type))
            luaL_error(L, "desired value is of the wrong type for this property");
         //
         self.before_edit();
         auto& list = prop->values;
         auto  size = list.size();
         if (i >= size) {
            if (i > size) {
               lua_warning(L, "index ", 1);
               const char* tostr = lua_tolstring(L, index_key, nullptr);
               lua_warning(L, tostr, 1);
               lua_warning(L, " is out of bounds; nil elements will be created between the end of the list and the new element", 0);
            }
            list.resize(i + 1);
         }
         set_papyrus_property_value(L, index_value, self, *prop, i);
         self.after_edit();
         return 0;
      }
   }
}
#pragma endregion

namespace {
   namespace _getters {
      luastackchange_t is_array(lua_State* L) {
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
      luastackchange_t type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
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
      luastackchange_t type(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
         luaL_argcheck(L, lua_isstring(L, 2), 2, "type (string) expected");
         //
         std::string type = cobb::trim(lua_tostring(L, 2));
         size_t      size = type.size();
         if (size < 0)
            luaL_error(L, "\"%s\" is not something Lua can recognize as a Papyrus typename", type.c_str());
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
            luaL_error(L, "\"%s\" is not something Lua can recognize as a Papyrus typename", type.c_str());
         if (prop->type == typeval)
            return 0;
         //
         self.before_edit();
         prop->set_type(*self.form, typeval);
         self.after_edit();
         return 0;
      }
      luastackchange_t value(lua_State* L) {
         DovahKitScriptVMPermissionInterface::verify_form_write_permissions();
         //
         auto& self = get_wrapper_for_thiscall<wrapper_t>(L);
         auto* prop = wrappers::papyrus_property::unwrap(self, true);
         if (prop == nullptr)
            luaL_error(L, "script property wrapper has no underlying object (deleted?)");
         __assume(prop != nullptr);
         //
         int  isnum;
         auto vt = lua_type(L, 2);
         if (prop->is_array()) {
            luaL_argcheck(L, vt == LUA_TUSERDATA || vt == LUA_TTABLE, 2, "attempted to set a Papyrus array property to the wrong type");
            auto* other = (wrapper*) editor_script::cast_to_class(L, 2, wrapper_t::array_collection_key);
            if (other) {
               if (self.is_equal(other)) // early-out on self-assignment
                  return 0;
               auto* source = wrappers::papyrus_property::unwrap(*other, false);
               luaL_argcheck(L, source, 2, "the provided property-value wrapper has no underlying object (deleted?)");
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
            luaL_argcheck(L, isnum, 2, "provided table or userdata did not return a valid result for its __len operator");
            for (int i = 1; i <= length; ++i) {
               lua_geti(L, 2, i);
               bool valid = property_scalar_value_typecheck(L, -1, prop->type);
               lua_pop(L, 1);
               if (!valid)
                  luaL_error(L, "provided array has a value of the wrong type at index %d", i);
            }
            //
            self.before_edit();
            prop->clear(*self.form);
            prop->values.resize(length);
            for (int i = 1; i <= length; ++i) {
               lua_geti(L, 2, i);
               set_papyrus_property_value(L, -1, self, *prop, i - 1);
               lua_pop(L, 1);
            }
            self.after_edit();
         } else {
            luaL_argcheck(L, property_scalar_value_typecheck(L, 2, prop->type), 2, "desired value is of the wrong type for this property");
            //
            self.before_edit();
            prop->clear(*self.form);
            prop->values.resize(1);
            set_papyrus_property_value(L, 2, self, *prop, 0);
            self.after_edit();
         }
         return 0;
      }
   }
}

namespace editor_script::wrappers {
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

   /*static*/ void wrapper_t::build_collection_metatables(lua_State* L) {
      define_collection_metatable(L, {
         .registry_key           = wrapper_t::array_collection_key,
         .garbage_collection     = &wrapper::__gc,
         //
         .get_collection_length  = &_collections::_array_values::get_collection_length,
         .lookup_item_by_index   = &_collections::_array_values::lookup_item_by_index,
         .member_function_insert = &_collections::_array_values::member_function_insert,
         .member_function_remove = &_collections::_array_values::member_function_remove,
         .set_item               = &_collections::_array_values::set_item,
      });
   }
}