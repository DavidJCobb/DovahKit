#include "./push_pull_indexed_parameter.h"
#include "lua.h"
#include "dovah/data/conditions/parameter_typeinfo.h"
#include "dovah/forms/Quest.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrappers/form/quest/alias.h"
#include "dovahscript/wrappers/form/form.h"

namespace dovahscript::api_helpers::conditions {
   extern std::expected<
      working_parameter,
      std::string_view
   > pull_indexed_parameter(
      lua_State* L,
      int pos,
      //
      const context_type& context,
      dovah::conditions::parameter_underlying_type underlying,
      const dovah::conditions::parameter_typeinfo* typeinfo
   ) {
      dovah::loaded_forms::components::conditions::working_parameter param;
      switch (underlying) {
         case dovah::conditions::parameter_underlying_type::alias:
            {
               auto* alias_wrapper = wrapper_from_stack<wrappers::quest_alias>(L, pos);
               if (!alias_wrapper)
                  return std::unexpected("quest alias expected");
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
               if (stub && typeinfo && !typeinfo->allows_form_type(stub->form_type)) {
                  return std::unexpected("invalid form type for this parameter");
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
                  return std::unexpected("this value is not representable in an unsigned 4-byte integer");
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

   extern void push_indexed_parameter(
      lua_State* L,
      const context_type& context,
      const dovah::conditions::parameter_typeinfo*       typeinfo,
      const dovah::conditions::parameter_underlying_type underlying,
      const working_parameter& param
   ) {
      if (std::holds_alternative<std::monostate>(param)) {
         lua_pushnil(L);
         return;
      }
      if (std::holds_alternative<uint32_t>(param)) {
         switch (underlying) {
            case dovah::conditions::parameter_underlying_type::alias:
               if (context.quest) {
                  int c = wrappers::quest_alias::wrap(L, context.quest, std::get<uint32_t>(param));
                  if (c > 0) {
                     if (c > 1)
                        lua_pop(L, c - 1);
                     return;
                  }
               }
               lua_pushnil(L);
               return;
            case dovah::conditions::parameter_underlying_type::int_unsigned:
            case dovah::conditions::parameter_underlying_type::quest_stage:
            default:
               lua_pushinteger(L, std::get<uint32_t>(param));
               return;
            case dovah::conditions::parameter_underlying_type::package_data:
               lua_pushinteger(L, std::get<uint32_t>(param)); // TODO: return a wrapped package data
               return;
         }
         std::unreachable();
         return;
      }
      if (std::holds_alternative<char>(param)) {
         lua_pushlstring(L, &std::get<char>(param), 1);
         return;
      }
      if (std::holds_alternative<float>(param)) {
         lua_pushnumber(L, std::get<float>(param));
         return;
      }
      if (std::holds_alternative<int32_t>(param)) {
         auto v = std::get<int32_t>(param);
         if (underlying == dovah::conditions::parameter_underlying_type::enumeration) {
            if (typeinfo && typeinfo->enumeration_info.has_value()) {
               const auto& enumeration = typeinfo->enumeration_info.value();
               for (size_t i = 0; i < enumeration.size; ++i) {
                  const auto& member = enumeration.members[i];
                  if (member.value == v) {
                     lua_pushstring(L, member.name.data());
                     return;
                  }
               }
            }
         }
         lua_pushinteger(L, v);
         return;
      }
      if (std::holds_alternative<dovah::form_stub*>(param)) {
         int c = push_native_object(std::get<dovah::form_stub*>(param));
         if (c > 0) {
            if (c > 1)
               lua_pop(L, c - 1);
            return;
         }
         lua_pushnil(L);
         return;
      }
      if (std::holds_alternative<std::string>(param)) {
         auto& data = std::get<std::string>(param);
         lua_pushstring(L, data.c_str());
         return;
      }

      lua_pushnil(L);
   }
}