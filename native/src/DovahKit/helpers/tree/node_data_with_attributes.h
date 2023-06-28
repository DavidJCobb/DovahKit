#pragma once
#include <array>
#include "./node_data_attribute.h"

namespace cobb {
   template<typename Data, node_data_attribute... Attributes>
   struct node_data_with_attributes {
      node_data_with_attributes() = delete;
      ~node_data_with_attributes() = delete;

      using data_type = Data;

      static constexpr const auto all_specified_attributes = std::array<node_data_attribute, sizeof...(Attributes)>{ Attributes... };

      template<node_data_attribute Attr>
      static constexpr const bool has_attribute = []() -> bool {
         for (auto attr : all_specified_attributes)
            if (attr == Attr)
               return true;
         return false;
      }();
   };
}