#pragma once
#include <type_traits>
#include "./is_accessor.h"
#include "./type_accessed_by.h"

namespace dovahscript::api_helpers::subobject_property_helpers::utils {
   template<typename T, typename AccessFunc>
   concept is_accessor_for_type = requires {
      requires is_accessor<AccessFunc>;
      requires (std::is_class_v<T> ?
         std::is_base_of_v<T, type_accessed_by<AccessFunc>>
      :
         std::is_same_v<T, type_accessed_by<AccessFunc>>
      );
   };
}