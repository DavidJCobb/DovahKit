#include "./property.h"
#include <variant>
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/data/papyrus/helpers/name_equals.h"
#include "dovah/forms/components/papyrus/attached_script.h"
#include "dovah/forms/components/papyrus/property.h"
#include "dovah/forms/components/papyrus/property_object_value.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/Quest.h"

#include "../../form/form.h"
#include "../../form/quest/alias.h"
#include "../papyrus.h"
#include "./collection_array_property_values.h"
#include "./script.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::papyrus_property;
   using wrapped_type = cls::wrapped_type;

   using root_wrapper   = wrappers::papyrus_root;
   using script_wrapper = wrappers::papyrus_script;

   using property_object_value = dovah::loaded_forms::components::papyrus::property_object_value;
   using property_type         = dovah::loaded_forms::components::papyrus::property_type;
}

wrapped_type* cls::unwrap(wrapper& w) {
   auto* script = wrappers::papyrus_script::unwrap(w);
   if (!script)
      return nullptr;

   size_t depth = w.parts.size();
   for (size_t i = 0; i < w.parts.size(); ++i) {
      if (w.parts[i].signature == wrapper_part_types::papyrus_property) {
         depth = i;
         break;
      }
   }
   if (depth >= w.parts.size())
      return nullptr;

   auto& list = script->properties;
   auto  i    = w.parts[depth].index;
   if (i >= list.size())
      return nullptr;
   return &list[i];
}

namespace {
   // empty   = not an alias
   // nullptr = zombie alias wrapper
   std::optional<dovah::loaded_forms::Alias*> _pull_alias(lua_State* L, int stack_pos) {
      if (auto* w = wrapper_from_stack<wrappers::quest_alias>(L, stack_pos)) {
         return wrappers::quest_alias::unwrap(*w);
      }
      return {};
   }
   
   // empty   = not a form
   // nullptr = zombie form wrapper
   std::optional<dovah::form_stub*> _pull_form(lua_State* L, int stack_pos) {
      if (auto* w = wrapper_from_stack<wrappers::form>(L, stack_pos)) {
         return w->stub;
      }
      return {};
   }

   enum class array_invalidity {
      contains_function,
      contains_table,
      contains_unknown,
      contains_userdata, // besides ones that are allowed
      contains_zombie_alias,
      contains_zombie_form,
      keys_out_of_range,
      mixed_types,
      non_integer_keys,
   };
   struct array_info {
      property_type type;
      size_t        max_index = 0;
   };
   static std::variant<array_info, array_invalidity> get_array_info(lua_State* L, int stack_pos) {
      stack_pos = lua_absindex(L, stack_pos);
      assert(lua_istable(L, stack_pos) || lua_isuserdata(L, stack_pos));

      std::optional<property_type> array_element_type;
      bool   has_aliases = false;
      bool   has_forms   = false;
      size_t max_index   = 0;
      
      lua_pushnil(L);  /* first key */
      while (lua_next(L, stack_pos) != 0) {
         if (!lua_isinteger(L, -2))
            return array_invalidity::non_integer_keys;
         auto i = lua_tointeger(L, -2);
         if (i <= 0)
            return array_invalidity::keys_out_of_range;
         if (i > max_index)
            max_index = i;

         constexpr const int element_stack_pos = -1;
         {
            property_type value_type;
            switch (lua_type(L, element_stack_pos)) {
               case LUA_TNIL:
               case LUA_TNONE:
                  value_type = property_type::object;
                  break;
               case LUA_TSTRING:
                  value_type = property_type::string;
                  break;
               case LUA_TBOOLEAN:
                  value_type = property_type::boolean;
                  break;
               case LUA_TFUNCTION:
                  return array_invalidity::contains_function;
               case LUA_TTABLE:
                  return array_invalidity::contains_table;
               case LUA_TUSERDATA:
                  if (auto alias_opt = _pull_alias(L, element_stack_pos); alias_opt.has_value()) {
                     if (!alias_opt.value())
                        return array_invalidity::contains_zombie_alias;
                     has_aliases = true;
                     value_type   = property_type::object;
                     break;
                  }
                  if (auto form_opt = _pull_form(L, element_stack_pos); form_opt.has_value()) {
                     if (!form_opt.value())
                        return array_invalidity::contains_zombie_form;
                     has_forms  = true;
                     value_type = property_type::object;
                     break;
                  }
                  return array_invalidity::contains_userdata;
               default:
                  return array_invalidity::contains_unknown;
            }
            if (array_element_type.has_value()) {
               if (array_element_type.value() != value_type)
                  return array_invalidity::mixed_types;
               if (has_aliases && has_forms)
                  return array_invalidity::mixed_types;
            } else {
               array_element_type = value_type;
            }
         }
         lua_pop(L, 1);
      }

      array_info info;
      info.max_index = max_index;
      if (!array_element_type.has_value()) {
         info.type = property_type::array_of_object;
      } else {
         info.type = dovah::loaded_forms::components::papyrus::array_property_type_for(array_element_type.value());
      }
      return info;
   }

   template<typename T>
   static std::vector<T> unpack_array(lua_State* L, int stack_pos, const array_info& info) {
      stack_pos = lua_absindex(L, stack_pos);
      assert(lua_istable(L, stack_pos) || lua_isuserdata(L, stack_pos));

      std::vector<T> out;
      out.resize(info.max_index);

      for (size_t i = 1; i <= info.max_index; ++i) {
         typename decltype(out)::reference dst = out[i - 1]; // can't just use `auto&` due to vector-of-bool

         lua_geti(L, stack_pos, i);
         constexpr const int element_stack_pos = -1;
         if constexpr (std::is_same_v<T, bool>) {
            dst = lua_toboolean(L, element_stack_pos);
         } else if constexpr (std::is_same_v<T, float>) {
            dst = lua_tonumber(L, element_stack_pos);
         } else if constexpr (std::is_same_v<T, int32_t>) {
            dst = lua_tointeger(L, element_stack_pos);
         } else if constexpr (std::is_same_v<T, std::string>) {
            dst = lua_tostring(L, element_stack_pos);
         } else if constexpr (std::is_same_v<T, property_object_value>) {
            if (!lua_isnoneornil(L, element_stack_pos)) {
               if (auto* w = wrapper_from_stack<wrappers::quest_alias>(L, element_stack_pos)) {
                  auto* alias = wrappers::quest_alias::unwrap(*w);
                  assert(!!alias);
                  dst.form.unmanaged_set(&alias->owner.stub);
                  dst.alias_id = alias->id;
               } else if (auto* form_wrapper = wrapper_from_stack<wrappers::form>(L, element_stack_pos)) {
                  assert(!!form_wrapper->stub);
                  dst.form.unmanaged_set(form_wrapper->stub);
               }
            }
         }
         lua_pop(L, 1);
      }

      return out;
   }
}

namespace {
   namespace _getters {
      int name(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            return 0;
         lua_pushstring(L, item->name.c_str());
         return 1;
      }
      int status(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            return 0;
         using enumeration = decltype(wrapped_type::status);
         switch (item->status) {
            case enumeration::unknown:
               lua_pushstring(L, "unknown"); // for legacy VMAD versions
               break;
            case enumeration::defined_locally:
               lua_pushstring(L, "local");
               break;
            case enumeration::defined_only_on_base:
               lua_pushstring(L, "inherited");
               break;
            case enumeration::inherited_and_removed:
               lua_pushstring(L, "removed");
               break;
            default:
               lua_pushnil(L);
               break;
         }
         return 1;
      }
      int value(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            return 0;

         switch (item->type()) {
            case property_type::none:
               lua_pushnil(L);
               return 1;
            case property_type::boolean:
               lua_pushboolean(L, std::get<bool>(item->value));
               return 1;
            case property_type::float32:
               lua_pushnumber(L, std::get<float>(item->value));
               return 1;
            case property_type::integer:
               lua_pushinteger(L, std::get<int32_t>(item->value));
               return 1;
            case property_type::string:
               lua_pushstring(L, std::get<std::string>(item->value).c_str());
               return 1;
            case property_type::object:
               {
                  auto& v = std::get<property_object_value>(item->value);
                  if (!v.form) {
                     lua_pushnil(L);
                     return 1;
                  }
                  if (v.alias_id != property_object_value::no_alias) {
                     return wrappers::quest_alias::wrap(L, v.form.get_form_stub(), v.alias_id);
                  }
                  return push_native_object(v.form);
               }
               return 0;
         }

         wrapper out = self;
         out.append_part(wrapper_part_types::papyrus_property_array);
         out.is_collection = true;
         return core::subsystems::userdata::get().push(L, out, wrappers::collections::papyrus_array_property_values.registry_key);
      }
   }
   namespace _setters {
      int name(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            cobb::lua::error(L, "papyrus_property wrapper has no underlying object (deleted?)");
         cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
         std::string_view name = lua_tostring(L, 2);

         if (dovah::papyrus::helpers::name_equals(name, item->name)) {
            //
            // Skip uniqueness checks and just apply the name (in case there are letter case changes).
            //
            self.before_edit();
            item->name = name;
            self.after_edit();
            return 0;
         }

         // Uniqueness check.
         auto* script = script_wrapper::unwrap(self);
         assert(script != nullptr);
         if (script->lookup_property(name)) {
            cobb::lua::argerror(L, 2, "another property with the requested name is already defined on this script");
         }

         self.before_edit();
         item->name = name;
         self.after_edit();
         return 0;
      }
      int status(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            cobb::lua::error(L, "papyrus_property wrapper has no underlying object (deleted?)");

         using enumeration = decltype(wrapped_type::status);
         enumeration value;
         {
            cobb::lua::argcheck(L, lua_isstring(L, 2), 2, "string expected");
            std::string_view v = lua_tostring(L, 2);
            if (v == "unknown") {
               value = enumeration::unknown;
            } else if (v == "local") {
               value = enumeration::defined_locally;
            } else if (v == "inherited") {
               value = enumeration::defined_only_on_base;
            } else if (v == "removed") {
               value = enumeration::inherited_and_removed;
            } else {
               cobb::lua::argerror(L, 2, "unrecognized value");
            }
         }

         assert(self.stub);
         if (!dovah::form_type_is_reference(self.stub->form_type)) {
            switch (value) {
               case enumeration::defined_only_on_base:
               case enumeration::inherited_and_removed:
                  cobb::lua::argerror(L, 2, "the only valid statuses for properties on scripts attached to non-refs are \"local\" and \"unknown\"");
                  break;
            }
         }

         self.before_edit();
         item->status = value;
         self.after_edit();
         return 0;
      }
      int value(lua_State* L) {
         core::subsystems::permissions::verify_form_write_permissions();

         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* item = cls::unwrap(self);
         if (!item)
            cobb::lua::error(L, "papyrus_property wrapper has no underlying object (deleted?)");

         decltype(wrapped_type::value) value;
         switch (lua_type(L, 2)) {
            case LUA_TNIL:
            case LUA_TNONE:
               value.emplace<std::monostate>();
               break;
            case LUA_TSTRING:
               value.emplace<std::string>(lua_tostring(L, 2));
               break;
            case LUA_TNUMBER:
               if (lua_isinteger(L, 2)) {
                  value.emplace<int32_t>(lua_tointeger(L, 2));
               } else {
                  value.emplace<float>(lua_tonumber(L, 2));
               }
               break;
            case LUA_TBOOLEAN:
               value.emplace<bool>(lua_toboolean(L, 2));
               break;
            case LUA_TUSERDATA:
               if (auto alias_opt = _pull_alias(L, 2);  alias_opt.has_value()) {
                  auto* alias = alias_opt.value();
                  cobb::lua::argcheck(L, !!alias, 2, "the provided value is a zombie alias wrapper");
                  auto& dst   = value.emplace<property_object_value>();
                  dst.form.unmanaged_set(&alias->owner.stub);
                  dst.alias_id = alias->id;
                  break;
               }
               if (auto form_opt = _pull_form(L, 2); form_opt.has_value()) {
                  auto* stub = form_opt.value();
                  cobb::lua::argcheck(L, !!stub, 2, "the provided value is a zombie form wrapper");
                  auto& dst = value.emplace<property_object_value>();
                  dst.form.unmanaged_set(stub);
                  break;
               }
               [[fallthrough]];
            case LUA_TTABLE:
               {
                  auto variant = get_array_info(L, 2);
                  if (std::holds_alternative<array_invalidity>(variant)) {
                     switch (std::get<array_invalidity>(variant)) {
                        case array_invalidity::contains_function:
                           cobb::lua::argerror(L, 2, "the provided array contains a function");
                        case array_invalidity::contains_table:
                           cobb::lua::argerror(L, 2, "the provided array contains a table");
                        case array_invalidity::contains_unknown:
                        case array_invalidity::contains_userdata:
                           cobb::lua::argerror(L, 2, "the provided array contains an invalid value");
                           cobb::lua::argerror(L, 2, "the provided array contains an invalid value");
                        case array_invalidity::contains_zombie_alias:
                        case array_invalidity::contains_zombie_form:
                           cobb::lua::argerror(L, 2, "the provided array contains a zombie object");
                        case array_invalidity::keys_out_of_range:
                           cobb::lua::argerror(L, 2, "the provided table contains keys that are zero or negative");
                        case array_invalidity::mixed_types:
                           cobb::lua::argerror(L, 2, "the provided array contains values of different types");
                        case array_invalidity::non_integer_keys:
                           cobb::lua::argerror(L, 2, "the provided table contains non-integer keys");
                     }
                     cobb::lua::argerror(L, 2, "invalid argument");
                  }
                  const auto& info = std::get<array_info>(variant);
                  switch (info.type) {
                     case property_type::array_of_object:
                        value = unpack_array<property_object_value>(L, 2, info);
                        break;
                     case property_type::array_of_boolean:
                        value = unpack_array<bool>(L, 2, info);
                        break;
                     case property_type::array_of_float32:
                        value = unpack_array<float>(L, 2, info);
                        break;
                     case property_type::array_of_integer:
                        value = unpack_array<int32_t>(L, 2, info);
                        break;
                     case property_type::array_of_string:
                        value = unpack_array<std::string>(L, 2, info);
                        break;
                  }
               }
               break;
         }

         auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();

         self.before_edit();
         item->set_value(*form, value);
         self.after_edit();
         return 0;
      }
   }
}
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "name",   &_getters::name },
      { "status", &_getters::status },
      { "value",  &_getters::value },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "name",   &_setters::name },
      { "status", &_setters::status },
      { "value",  &_setters::value },
   };
}