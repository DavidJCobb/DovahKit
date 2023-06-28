#pragma once
#include "./is_node_data_with_attributes.h"

namespace cobb::impl::_node {
   template<typename T>
   struct strip_node_data_options {
      using type = T;
   };

   template<is_node_data_with_attributes T>
   struct strip_node_data_options<T> {
      using type = typename T::data_type;
   };
}