#pragma once
#include <string_view>
#include "helpers/type_traits/is_std_array.h"
#include "dovah/form_reference_t.h"
#include "dovah/form_stub.h"
#include "dovah/form_types.h"
#include "../common_lambdas.h"
#include "../property_definition.h"
#include "../utils/is_accessor_for_type.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<auto FormTypeOrTypes, bool AllowNone = true>
      requires (
         std::is_same_v<decltype(FormTypeOrTypes), dovah::form_type> ||
         cobb::is_std_array<std::decay_t<decltype(FormTypeOrTypes)>>
      )
   struct form_property {
      form_property() = delete;
      public:
         static constexpr const bool allows_multiple_form_types   = cobb::is_std_array<std::decay_t<decltype(FormTypeOrTypes)>>;
         static constexpr const bool form_types_are_unconstrained = []() {
            if constexpr (allows_multiple_form_types) {
               return FormTypeOrTypes.size() == 0;
            } else {
               return FormTypeOrTypes == dovah::form_type::none;
            }
         }();

         static std::string_view check_lua_value(lua_State* L, int pos) {
            if (lua_isnoneornil(L, pos)) {
               if constexpr (AllowNone) {
                  return "value must be a form (got nil)"; // TODO: print required type names
               } else {
                  return "";
               }
            }
            auto* w = wrapper_from_stack<wrappers::form>(L, pos);
            if (!w || !w->stub)
               return "value must be a form";
            if constexpr (!form_types_are_unconstrained) {
               const auto ft = w->stub->form_type;
               if constexpr (allows_multiple_form_types) {
                  for (auto allowed : FormTypeOrTypes) {
                     if (allowed == ft)
                        return {};
                     if (allowed == dovah::form_type::reference && dovah::form_type_is_reference(ft))
                        return {};
                  }
               } else {
                  if (ft == FormTypeOrTypes)
                     return {};
                  if constexpr (FormTypeOrTypes == dovah::form_type::reference)
                     if (dovah::form_type_is_reference(ft))
                        return {};
               }
               return "value must be a form of the correct type"; // TODO: print required type name
            }
            return {};
         }

      public:
         template<typename AccessFunc>
            requires utils::is_accessor_for_type<dovah::form_reference_t, AccessFunc>
         static consteval auto define(std::string_view name, AccessFunc a) {
            return property_definition{
               .name   = name,
               .access = a,
               .check  = &check_lua_value,
               .pull   = &pull::form_stub,
               .push   = &push::form_reference,
               .default_value = (dovah::form_stub*)nullptr,
            };
         }
   };
}