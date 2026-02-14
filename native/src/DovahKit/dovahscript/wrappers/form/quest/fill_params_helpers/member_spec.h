#pragma once
#include <string_view>
struct lua_State;

namespace dovahscript::api_helpers::fill_params_helpers {
   template<typename AliasType, typename FillParams>
   struct member_spec {
      std::string_view name;
      bool optional_for_overwrite = false;
      struct {
         void(*complain)(const AliasType&, lua_State* L, int stack_pos);
         bool(*silently)(const AliasType&, lua_State* L, int stack_pos);
      } validators;
      void(*write)(AliasType&, FillParams&, lua_State* L, int stack_pos);
   };
}