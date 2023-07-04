#pragma once
#include <type_traits>
#include <tuple>
#include <variant>
#include "../tuples/prepend.h"
#include "../tuples/unpack_types_into.h"
#include "./node.h"

namespace cobb {
   template<typename Node, bool AllowEmpty = false>
   using node_data_variant = std::conditional_t<
      AllowEmpty,
      cobb::tuples::unpack_types_into<
         cobb::tuples::prepend<
            std::monostate,
            typename Node::all_data_types::as_tuple
         >,
         std::variant
      >,
      cobb::tuples::unpack_types_into<
         typename Node::all_data_types::as_tuple,
         std::variant
      >
   >;

   template<typename Variant, typename Node>
   Variant node_to_variant(const Node& node) {
      using type_list = typename Node::all_data_types;

      Variant out;
      type_list::template for_each_until_true([&node, &out]<typename Current>() {
         if (auto* casted = node.as<Current>()) {
            out = casted->data;
            return true;
         }
         return false;
      });
      return out;
   }

   template<typename Node, typename Variant>
   Node* node_from_variant(const Variant& v) {
      using type_list = typename Node::all_data_types;

      Node* out = nullptr;
      type_list::template for_each_until_true([&v, &out]<typename Current>() {
         if (std::holds_alternative<Current>(v)) {
            out = Node::template from_data<Current>(std::get<Current>(v));
            return true;
         }
         return false;
      });
      return out;
   }
}
