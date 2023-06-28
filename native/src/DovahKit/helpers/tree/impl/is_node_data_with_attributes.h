#pragma once
#include <type_traits>
#include "../node_data_attribute.h"
#include "../node_data_with_attributes.h"
#include "../../arrays/unpack_values_as_NTTPs.h"

namespace cobb::impl::_node {
   template<typename T>
   concept is_node_data_with_attributes = requires {
      requires std::is_same_v<
         T,
         typename cobb::arrays::unpack_values_as_nttp<T::all_specified_attributes, typename T::data_type>::template into<node_data_with_attributes>
      > == true;
   };
}