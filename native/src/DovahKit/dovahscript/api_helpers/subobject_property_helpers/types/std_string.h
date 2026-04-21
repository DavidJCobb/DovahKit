#pragma once
#include <string>
#include <string_view>
#include "../common_lambdas.h"
#include "../property_definition.h"
#include "../utils/is_accessor_for_type.h"

namespace dovahscript::api_helpers::subobject_property_helpers {
   // Dummy struct in case we ever need to add template parameters, and for 
   // consistency with cases that need template parameters.
   struct std_string_property {
      std_string_property() = delete;

      template<typename AccessFunc>
         requires utils::is_accessor_for_type<std::string, AccessFunc>
      static consteval auto define(std::string_view name, AccessFunc a) {
         return property_definition{
            .name   = name,
            .access = a,
            .check  = &checks::string,
            .pull   = &pull::string_view,
            .push   = &push::string,
            .default_value = std::string_view{},
         };
      }
   };
}