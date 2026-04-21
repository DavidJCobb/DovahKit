#include "./pull_condition_parameter_from_lua.h"
#include "lua.h"
#include "dovah/data/conditions/all_function_info.h"
#include "dovah/forms/components/conditions/context.h"
#include "dovah/forms/Quest.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/wrapper.h"
#include "dovahscript/wrappers/form_components/condition.h"
#include "dovahscript/wrappers/form/form.h"
#include "dovahscript/wrappers/form/quest/alias.h"

#define FOR_EACH_EVENT_FUNCTION_ID(DO) \
   DO(GetIsID) \
   DO(IsInList) \
   DO(GetValue) \
   DO(HasKeyword) \
   DO(GetItemValue) \

namespace dovahscript {
   extern std::expected<
      dovah::loaded_forms::components::conditions::working_parameter,
      std::string_view
   > pull_condition_parameter_from_lua(
      lua_State* L,
      int pos,
      wrapper& self,
      const dovah::loaded_forms::components::conditions::working_condition& working,
      int which_parameter
   ) {
      auto* form = self.get_loaded_form_data<dovah::loaded_forms::Form>();

      dovah::loaded_forms::components::conditions::working_parameter param;

      auto underlying = working.get_argument_underlying_type(which_parameter);
      switch (underlying) {
         case dovah::conditions::parameter_underlying_type::alias:
            {
               auto* alias_wrapper = wrapper_from_stack<wrappers::quest_alias>(L, pos);
               if (!alias_wrapper)
                  return std::unexpected("quest alias expected");
               auto context = wrappers::condition::context_of(self);
               if (!context.quest)
                  return std::unexpected("this condition has no owning quest and so cannot refer to a quest alias");
               auto* alias = wrappers::quest_alias::unwrap(*alias_wrapper);
               if (!alias)
                  return std::unexpected("passed-in quest alias is missing (deleted?)");
               if (&alias->owner.stub != context.quest)
                  return std::unexpected("the passed-in quest alias does not belong to this condition's owning quest");
               param.emplace<uint32_t>(alias->id);
            }
            break;
         case dovah::conditions::parameter_underlying_type::character:
            if (!lua_isstring(L, pos))
               return std::unexpected("character (string of size 1) expected");
            {
               std::string_view v = lua_tostring(L, pos);
               if (v.size() != 1)
                  return std::unexpected("character (string of size 1) expected");
               param.emplace<char>(v[0]);
            }
            break;
         case dovah::conditions::parameter_underlying_type::enumeration:
            if (!lua_isstring(L, pos))
               return std::unexpected("string expected");
            {
               auto* func = dovah::conditions::function_info_by_id(working.function);
               if (!func)
                  return std::unexpected("internal error: unable to find internal data for this condition function, so we don't know what values are valid here");
               auto* typeinfo = func->argument_types[which_parameter];
               if (!typeinfo)
                  return std::unexpected("internal error: unable to find internal data (typeinfo) for this parameter type, so we don't know what values are valid here");
               if (!typeinfo->enumeration_info.has_value())
                  return std::unexpected("internal error: unable to find internal data (enumeration data) for this parameter type, so we don't know what values are valid here");

               std::optional<int32_t> value;
               {
                  const auto& enumeration = typeinfo->enumeration_info.value();
                  std::string name        = lua_tostring(L, pos);
                  for (size_t i = 0; i < enumeration.size; ++i) {
                     const auto& member = enumeration.members[i];
                     if (member.name == name) {
                        value = member.value;
                        break;
                     }
                  }
                  if (!value.has_value())
                     return std::unexpected("unrecognized value");
               }
               param.emplace<int32_t>(value.value());
            }
            break;
         case dovah::conditions::parameter_underlying_type::float32:
            if (!lua_isnumber(L, pos))
               return std::unexpected("number expected");
            param.emplace<float>(lua_tonumber(L, pos));
            break;
         case dovah::conditions::parameter_underlying_type::form:
            {
               dovah::form_stub* stub = nullptr;
               if (!lua_isnoneornil(L, pos)) {
                  auto* other = wrapper_from_stack<wrappers::form>(L, pos);
                  if (!other)
                     return std::unexpected("form or nil expected");
                  stub = other->stub;
               }
               if (stub) {
                  auto* func = dovah::conditions::function_info_by_id(working.function);
                  if (func) {
                     if (auto* typeinfo = func->argument_types[which_parameter]) {
                        if (!typeinfo->allows_form_type(stub->form_type)) {
                           return std::unexpected("invalid form type for this parameter");
                        }
                     }
                  }
               }
               param.emplace<dovah::form_stub*>(stub);
            }
            break;
         case dovah::conditions::parameter_underlying_type::int_signed:
            if (!lua_isinteger(L, pos))
               return std::unexpected("integer expected");
            {
               auto v = lua_tointeger(L, pos);
               if (v < std::numeric_limits<int32_t>::lowest() || v > std::numeric_limits<int32_t>::max())
                  return std::unexpected("this value is not representable in a signed 4-byte integer");
               param.emplace<int32_t>(v);
            }
            break;
         case dovah::conditions::parameter_underlying_type::int_unsigned:
            if (!lua_isinteger(L, pos))
               return std::unexpected("unsigned integer expected");
            {
               auto v = lua_tointeger(L, pos);
               if (v < 0)
                  return std::unexpected("unsigned integer expected");
               if (v > std::numeric_limits<int32_t>::max())
                  return std::unexpected("this value is not representable in a signed 4-byte integer");
               param.emplace<uint32_t>(v);
            }
            break;
         case dovah::conditions::parameter_underlying_type::package_data:
            return std::unexpected("package-data parameters are not yet implemented in Dovahscript"); // TODO
            break;
         case dovah::conditions::parameter_underlying_type::quest_stage:
            if (!lua_isinteger(L, pos))
               return std::unexpected("unsigned integer expected");
            {
               using value_type = decltype(dovah::loaded_forms::Quest::Stage::index);

               auto v = lua_tointeger(L, pos);
               if (v < 0)
                  return std::unexpected("unsigned integer expected");
               if (v > std::numeric_limits<value_type>::max())
                  return std::unexpected("this value is too high to be a valid quest stage");
               param.emplace<uint32_t>(v);
            }
            break;
         case dovah::conditions::parameter_underlying_type::string:
            if (!lua_isstring(L, pos))
               return std::unexpected("string expected");
            param.emplace<std::string>(lua_tostring(L, pos));
            break;
      }

      return param;
   }

   extern std::expected<dovah::conditions::event_function::type, std::string_view> pull_condition_event_function_from_lua(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected");
      std::string_view v = lua_tostring(L, pos);
      #define CASE(name, ...) if (v == #name) return dovah::conditions::event_function::name;
      FOR_EACH_EVENT_FUNCTION_ID(CASE)
      #undef CASE
      return std::unexpected("unrecognized event function name");
   }
   extern std::expected<uint16_t, std::string_view> pull_condition_event_member_from_lua(lua_State* L, int pos) {
      if (!lua_isstring(L, pos))
         return std::unexpected("string expected");
      std::string_view name = lua_tostring(L, pos);
      if (name.size() != 2)
         return std::unexpected("argument is not an event member signature");
      if constexpr (std::endian::native == std::endian::little) {
         return name[1] | ((uint16_t)name[0] << 8);
      } else {
         return name[0] | ((uint16_t)name[1] << 8);
      }
   }
   extern std::expected<dovah::form_stub*, std::string_view> pull_condition_event_form_from_lua(lua_State* L, int pos) {
      if (lua_isnoneornil(L, pos))
         return nullptr;
      auto* other = wrapper_from_stack<wrappers::form>(L, pos);
      if (!other)
         return std::unexpected("form or nil expected");
      return other->stub;
   }
}