#include "./ref_alias_fill_params_linked_ref.h"
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
   using cls       = wrappers::quest_ref_alias_fill_params_linked_ref;
   using alias_cls = wrappers::quest_ref_alias;

   using member_spec = dovahscript::api_helpers::fill_params_helpers::member_spec<
      wrappers::quest_alias::wrapped_type,
      cls::wrapped_type
   >;
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
      .name       = "alias",
      .validators = {
         .complain = api_helpers::fill_params_helpers::validate_earlier_alias<true,  false, alias_cls::wrapped_type::alias_type::reference>,
         .silently = api_helpers::fill_params_helpers::validate_earlier_alias<false, false, alias_cls::wrapped_type::alias_type::reference>,
      },
      .write = [](wrappers::quest_alias::wrapped_type& alias, cls::wrapped_type& fill, lua_State* L, int stack_pos) {
         stack_pos = lua_absindex(L, stack_pos);
         if (lua_isnoneornil(L, stack_pos)) {
            fill.alias = -1;
            return;
         }
         auto* value_wrap  = (wrapper*)dovahscript::classes::cast_to_class(L, stack_pos, wrappers::quest_ref_alias::metatable_key);
         auto* value_alias = wrappers::quest_alias::unwrap(*value_wrap);
         fill.alias = value_alias->id;
      }
   },
};
constexpr const auto& member_spec_by_name(std::string_view name) noexcept {
   for (auto& n : members)
      if (n.name == name)
         return n;
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
      int alias(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* fill = cls::unwrap(self);
         if (!fill)
            return 0;
         if (fill->alias == (dovah::loaded_forms::alias_id_t)-1) {
            lua_pushnil(L);
            return 1;
         }
         auto* alias = alias_cls::unwrap(self);
         return alias_cls::wrap(L, &alias->owner.stub, fill->alias);
      }
      int type(lua_State* L) {
         auto& self = get_wrapper_for_thiscall<cls>(L);
         auto* fill = cls::unwrap(self);
         if (!fill)
            return 0;
         lua_pushstring(L, "linked ref of");
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
      { "alias", &_getters::alias },
      { "type",  &_getters::type },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "alias", &_setters::_member_setter<member_spec_by_name("alias")> },
   };
}