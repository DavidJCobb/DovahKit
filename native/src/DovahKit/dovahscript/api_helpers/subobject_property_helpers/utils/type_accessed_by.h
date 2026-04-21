#pragma once
#include "./is_accessor.h"

namespace dovahscript::api_helpers::subobject_property_helpers::utils {
   template<typename AccessFunc>
      requires is_accessor<AccessFunc>
   using type_accessed_by = std::decay_t<typename cobb::function_traits<AccessFunc>::return_type>;
}