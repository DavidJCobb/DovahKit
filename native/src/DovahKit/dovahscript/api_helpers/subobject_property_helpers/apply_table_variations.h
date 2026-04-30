#pragma once
#include "lua.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/wrapper.h"
#include "dovahscript/wrappers/base.h"
#include "./apply_table.h"
#include "./verify_table.h"
#include "./verify_table_late.h"
#include "./utils/property_list_has_late_checks.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   //
   // UnwrapFunc must take a lua_State* as an argument and return a Subobject&
   //
   template<typename Wrapper, auto UnwrapFunc, const auto& PropertyList, bool IsAssign>
   int apply_table_via_lua_method(lua_State* L) {
      using subobject_type = std::decay_t<typename cobb::function_traits<std::decay_t<decltype(UnwrapFunc)>>::return_type>;

      constexpr const int argument_stack_pos = 2;

      core::subsystems::permissions::verify_form_write_permissions();
      //
      wrapper&        self      = get_wrapper_for_thiscall<Wrapper>(L);
      subobject_type& subobject = UnwrapFunc(L);
      {
         auto message = verify_table<subobject_type, IsAssign, PropertyList>(L, argument_stack_pos);
         if (!message.empty())
            cobb::lua::error(L, "bad argument: %s", message.data());
      }
      if constexpr (utils::property_list_has_late_checks<PropertyList>) {
         verify_table_late<subobject_type, IsAssign, PropertyList>(L, argument_stack_pos, *self.form, &subobject);
      }
      self.before_edit();
      subobject_property_helpers::apply_table<subobject_type, IsAssign, PropertyList>(L, argument_stack_pos, *self.form, subobject);
      self.after_edit();
      return 0;
   }

   template<typename Wrapper, auto UnwrapFunc, const auto& PropertyList>
   auto assign_table_via_lua_method = apply_table_via_lua_method<Wrapper, UnwrapFunc, PropertyList, true>;

   template<typename Wrapper, auto UnwrapFunc, const auto& PropertyList>
   auto overwrite_with_table_via_lua_method = apply_table_via_lua_method<Wrapper, UnwrapFunc, PropertyList, false>;
}