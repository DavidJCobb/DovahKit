#pragma once
#include "../node_type.h"
namespace IdleAnimationFormsModel_impl {
   class graph_node;
   class action_node;
   class idle_node;
   class loose_action_parent_node;
   class loose_idle_parent_node;
}

namespace IdleAnimationFormsModel_impl::utils {
   namespace impl {
      template<node_type> struct node_type_from_enum;
      template<> struct node_type_from_enum<node_type::action> { using type = action_node; };
      template<> struct node_type_from_enum<node_type::graph> { using type = graph_node; };
      template<> struct node_type_from_enum<node_type::idle> { using type = idle_node; };
      template<> struct node_type_from_enum<node_type::loose_action_container> { using type = loose_action_parent_node; };
      template<> struct node_type_from_enum<node_type::loose_idle_container> { using type = loose_idle_parent_node; };
   }
   template<node_type Enum> using node_type_from_enum = typename impl::node_type_from_enum<Enum>::type;
}