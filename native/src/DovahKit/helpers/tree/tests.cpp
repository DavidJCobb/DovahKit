#include "./node.h"

namespace {
   static_assert(
      []() -> bool {
         using test_node_type = cobb::node<
            int,
            double,
            cobb::node_data_with_attributes<
               char,
               cobb::node_data_attribute::leaf
            >
         >;

         auto* node = test_node_type::make<int>(5);
         bool result = node->can_have_children();
         delete node;
         return result;
      }(),
      "Assert: non-leaf node allows children."
   );

   static_assert(
      []() -> bool {
         using test_node_type = cobb::node<
            int,
            cobb::node_data_with_attributes<
               char,
               cobb::node_data_attribute::leaf
            >
         >;

         auto* node = test_node_type::make<char>(5);
         bool result = node->can_have_children();
         delete node;
         return !result;
      }(),
      "Assert: leaf node does not allow children."
   );
}