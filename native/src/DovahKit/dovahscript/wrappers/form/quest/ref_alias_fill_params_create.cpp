#include "./ref_alias_fill_params_create.h"
#include "helpers/lua/error.h"
#include "dovahscript/api_helpers/fail_table_if_expandos.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/core/classes.h"
#include "dovahscript/pull_native_object.h"
#include "dovahscript/push_native_object.h"
#include "dovahscript/wrapper.h"

#include "dovah/form_stub.h"
#include "dovah/forms/Quest.h"
#include "dovah/forms/structs/alias_fill_params.h"
#include "./ref_alias.h"
#include "../form.h"
#include "./fill_params_helpers/member_setter.h"
#include "./fill_params_helpers/member_spec.h"
#include "./fill_params_helpers/overwrite_with_table.h"
#include "./fill_params_helpers/validate_earlier_alias.h"
#include "./fill_params_helpers/validate_table.h"

namespace {
   using namespace dovahscript;
   using cls       = wrappers::quest_ref_alias_fill_params_create;
   using alias_cls = wrappers::quest_ref_alias;

   using member_spec = dovahscript::api_helpers::fill_params_helpers::member_spec<
      wrappers::quest_alias::wrapped_type,
      cls::wrapped_type
   >;
   
   constexpr const auto difficulty_names = std::array{
      std::string_view{"easy"},
      std::string_view{"medium"},
      std::string_view{"hard"},
      std::string_view{"very hard"},
      std::string_view{"none"},
   };
}

/*static*/ cls::wrapped_type* cls::unwrap(wrapper& self) {
   auto* alias = alias_cls::unwrap(self);
   if (!alias)
      return nullptr;
   if (alias->type != alias_cls::wrapped_type::alias_type::reference)
      return nullptr;
   return std::get_if<cls::wrapped_type>(&alias->fill_params);
}

constexpr const auto members = std::array{
   member_spec{
      .name       = "base_form",
      .validators = {
         .complain = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            stack_pos = lua_absindex(L, stack_pos);
            if (lua_isnoneornil(L, stack_pos))
               return;
            auto* value = pull_form_stub_argument(L, stack_pos);
            if (!value)
               cobb::lua::argerror(L, stack_pos, "zombie argument");
            if (value && !dovah::form_type_is_base_form(value->form_type))
               cobb::lua::argerror(L, stack_pos, "expected a base form");
         },
         .silently = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            stack_pos = lua_absindex(L, stack_pos);
            if (lua_isnoneornil(L, stack_pos))
               return true;
            auto* wrap = wrapper_from_stack<wrappers::form>(L, stack_pos);
            if (!wrap || !wrap->stub || !dovah::form_type_is_base_form(wrap->stub->form_type))
               return false;
            return true;
         },
      },
      .write = [](wrappers::quest_alias::wrapped_type& alias, cls::wrapped_type& fill, lua_State* L, int stack_pos) {
         stack_pos = lua_absindex(L, stack_pos);
         dovah::form_stub* stub = nullptr;
         if (!lua_isnoneornil(L, stack_pos))
            stub = wrapper_from_stack<wrappers::form>(L, stack_pos)->stub;
         fill.base_form.set(alias.owner, stub);
      }
   },
   member_spec{
      .name       = "create_at",
      .validators = {
         .complain = api_helpers::fill_params_helpers::validate_earlier_alias<true,  false, alias_cls::wrapped_type::alias_type::reference>,
         .silently = api_helpers::fill_params_helpers::validate_earlier_alias<false, false, alias_cls::wrapped_type::alias_type::reference>,
      },
      .write = [](wrappers::quest_alias::wrapped_type& alias, cls::wrapped_type& fill, lua_State* L, int stack_pos) {
         stack_pos = lua_absindex(L, stack_pos);
         if (lua_isnoneornil(L, stack_pos)) {
            fill.at_reference.alias = -1;
            return;
         }
         auto* value_wrap  = (wrapper*)dovahscript::classes::cast_to_class(L, stack_pos, wrappers::quest_ref_alias::metatable_key);
         auto* value_alias = wrappers::quest_alias::unwrap(*value_wrap);
         fill.at_reference.alias = value_alias->id;
      }
   },
   member_spec{
      .name       = "create_in_inventory",
      .optional_for_overwrite = true,
      .validators = {
         .complain = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            stack_pos = lua_absindex(L, stack_pos);
            cobb::lua::argcheck(L, lua_isboolean(L, stack_pos), stack_pos, "boolean expected");
         },
         .silently = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            return lua_isboolean(L, stack_pos);
         },
      },
      .write = [](wrappers::quest_alias::wrapped_type& alias, cls::wrapped_type& fill, lua_State* L, int stack_pos) {
         fill.at_reference.place_in_inventory = lua_toboolean(L, stack_pos);
      }
   },
   member_spec{
      .name       = "difficulty",
      .optional_for_overwrite = true,
      .validators = {
         .complain = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            std::string_view v = lua_tostring(L, stack_pos);
            for (size_t i = 0; i < difficulty_names.size(); ++i)
               if (v == difficulty_names[i])
                  return;
            cobb::lua::argerror(L, 2, "unrecognized value");
         },
         .silently = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            std::string_view v = lua_tostring(L, stack_pos);
            for (size_t i = 0; i < difficulty_names.size(); ++i)
               if (v == difficulty_names[i])
                  return true;
            return false;
         },
      },
      .write = [](wrappers::quest_alias::wrapped_type& alias, cls::wrapped_type& fill, lua_State* L, int stack_pos) {
         std::string_view v = lua_tostring(L, stack_pos);
         for (size_t i = 0; i < difficulty_names.size(); ++i) {
            if (difficulty_names[i] == v) {
               fill.difficulty = i;
               return;
            }
         }
      }
   },
   member_spec{
      .name       = "initially_disabled",
      .optional_for_overwrite = true,
      .validators = {
         .complain = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            stack_pos = lua_absindex(L, stack_pos);
            cobb::lua::argcheck(L, lua_isboolean(L, stack_pos), stack_pos, "boolean expected");
         },
         .silently = [](const wrappers::quest_alias::wrapped_type& alias, lua_State* L, int stack_pos) {
            return lua_isboolean(L, stack_pos);
         },
      },
      .write = [](wrappers::quest_alias::wrapped_type& alias, cls::wrapped_type& fill, lua_State* L, int stack_pos) {
         cobb::edit_bit(alias.flags, alias_cls::wrapped_type::flag::initially_disabled, lua_toboolean(L, stack_pos));
      }
   },
};
constexpr const auto& member_spec_by_name(std::string_view name) noexcept {
   for (auto& n : members)
      if (n.name == name)
         return n;
   // don't warn about `throw` in `noexcept`; we're using it to make constant evaluation fail deliberately
   #pragma warning(suppress:4297)
   throw;
}

/*static*/ bool cls::validate_table(quest_ref_alias::wrapped_type& self_alias, lua_State* L, int stack_pos) {
   return dovahscript::api_helpers::fill_params_helpers::validate_table<members>(self_alias, L, stack_pos);
}
/*static*/ void cls::overwrite_with_table(quest_ref_alias::wrapped_type& self_alias, wrapped_type& fill, lua_State* L, int stack_pos) {
   return dovahscript::api_helpers::fill_params_helpers::overwrite_with_table<members>(self_alias, fill, L, stack_pos);
}

namespace {
   namespace _getters {
      template<uint32_t Flag>
      int _flag(lua_State* L) {
         auto& self  = get_wrapper_for_thiscall<cls>(L);
         auto* alias = alias_cls::unwrap(self);
         if (alias == nullptr)
            return 0;
         lua_pushboolean(L, !!(alias->flags & Flag));
         return 1;
      }

      int base_form(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* fill = cls::unwrap(self);
         if (!fill)
            return 0;
         return push_native_object(fill->base_form);
      }
      int create_at(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* fill = cls::unwrap(self);
         if (!fill)
            return 0;
         if (fill->at_reference.alias == (dovah::loaded_forms::alias_id_t)-1) {
            lua_pushnil(L);
            return 1;
         }
         auto* alias = alias_cls::unwrap(self);
         return alias_cls::wrap(L, &alias->owner.stub, fill->at_reference.alias);
      }
      int create_in_inventory(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* fill = cls::unwrap(self);
         if (!fill)
            return 0;
         lua_pushboolean(L, fill->at_reference.place_in_inventory);
         return 1;
      }
      int difficulty(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* fill = cls::unwrap(self);
         if (!fill)
            return 0;
         if (fill->difficulty < difficulty_names.size()) {
            lua_pushstring(L, difficulty_names[fill->difficulty].data());
            return 1;
         }
         return 0;
      }
      int type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* fill = cls::unwrap(self);
         if (!fill)
            return 0;
         lua_pushstring(L, "create");
         return 1;
      }
   }
   namespace _setters {
      template<const member_spec& Member>
      int _member_setter(lua_State* L) {
         return api_helpers::fill_params_helpers::member_setter<cls, Member>(L);
      }
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   /*static*/ cls::method_list_t cls::metatable_getters = {
      #pragma region Flags
         { "initially_disabled", &_getters::_flag<alias_cls::wrapped_type::flag::initially_disabled> },
      #pragma endregion
      { "base_form",           &_getters::base_form },
      { "create_at",           &_getters::create_at },
      { "create_in_inventory", &_getters::create_in_inventory },
      { "difficulty",          &_getters::difficulty },
      { "type",                &_getters::type },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "base_form",           &_setters::_member_setter<member_spec_by_name("base_form")> },
      { "create_at",           &_setters::_member_setter<member_spec_by_name("create_at")> },
      { "create_in_inventory", &_setters::_member_setter<member_spec_by_name("create_in_inventory")> },
      { "difficulty",          &_setters::_member_setter<member_spec_by_name("difficulty")> },
      { "initially_disabled",  &_setters::_member_setter<member_spec_by_name("initially_disabled")> },
   };
}