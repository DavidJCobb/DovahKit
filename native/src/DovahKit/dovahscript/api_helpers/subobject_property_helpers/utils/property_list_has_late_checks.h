#pragma once
#include "helpers/tuples/for_each_value.h"

namespace dovahscript::api_helpers::subobject_property_helpers::utils {
   template<const auto& PropertyList>
   constexpr const bool property_list_has_late_checks = []() consteval -> bool {
      bool any = false;
      cobb::tuples::for_each_value(PropertyList, [&any](const auto& dfn) {
         if (dfn.late_check)
            any = true;
      });
      return any;
   }();
}