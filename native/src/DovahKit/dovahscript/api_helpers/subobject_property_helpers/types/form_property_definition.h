#pragma once
#include <string_view>
#include "helpers/type_traits/is_std_array.h"
#include "dovah/form_reference_t.h"
#include "dovah/form_stub.h"
#include "dovah/form_types.h"
#include "../common_lambdas.h"
#include "../property_definition.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   template<auto FormTypeOrTypes>
      requires (
         std::is_same_v<decltype(FormTypeOrTypes), dovah::form_type> ||
         cobb::is_std_array<std::decay_t<decltype(FormTypeOrTypes)>>
      )
   struct form_property_of_type {
      form_property_of_type() = delete;
      ~form_property_of_type() = delete;
      public:
         static constexpr const bool allows_multiple_form_types   = cobb::is_std_array<std::decay_t<decltype(FormTypeOrTypes)>>;
         static constexpr const bool form_types_are_unconstrained = []() {
            if constexpr (allows_multiple_form_types) {
               return FormTypeOrTypes.size() == 0;
            } else {
               return FormTypeOrTypes == dovah::form_type::none;
            }
         }();

      //protected:
         static std::string_view _check(lua_State* L, int pos) {
            auto* w = wrapper_from_stack<wrappers::form>(L, pos);
            if (!w || !w->stub)
               return "value must be a form of type `idle`";
            if constexpr (!form_types_are_unconstrained) {
               auto ft = w->stub->form_type;
               if constexpr (allows_multiple_form_types) {
                  for (auto allowed : FormTypeOrTypes)
                     if (allowed == ft)
                        return {};
                  return "value must be a form of the correct type"; // TODO: print required type names
               } else {
                  if (w->stub->form_type != FormTypeOrTypes)
                     return "value must be a form of the correct type"; // TODO: print required type name
               }
            }
            return {};
         }

      public:
         template<typename AccessFunc>
         struct define : public property_definition<
            AccessFunc,
            decltype(&_check),
            decltype(&pull::form_stub),
            decltype(&push::form_reference)
         > {
            public:
               constexpr define(std::string_view name, AccessFunc a) {
                  this->name   = name;
                  this->access = a;
                  this->check  = &_check;
                  this->pull   = &pull::form_stub;
                  this->push   = &push::form_reference;
               }
         };

         template<typename AccessFunc>
         define(std::string_view, AccessFunc) -> define<AccessFunc>;
   };

   template<auto FormTypeOrTypes>
   consteval auto define_form_property(std::string_view name, auto accessor) {
      return form_property_of_type<FormTypeOrTypes>::define(name, accessor);
   }
}