#pragma once
#include "../node_type.h"
namespace IdleAnimationFormsModel_impl {
   class graph_node;
   class action_node;
   class idle_node;
   class loose_container_node;
}

namespace IdleAnimationFormsModel_impl::utils {
   namespace impl {
      template<node_type> struct node_type_from_enum;
   }
   template<node_type Enum> using node_type_from_enum = typename impl::node_type_from_enum<Enum>::type;

   // ... implementation below ...

   namespace impl {
      #define MAKE_MAPPING(name) \
         template<> struct node_type_from_enum<node_type::name> { using type = name##_node; };
      #include "../node_type_x_macro.define.h"
      X_NODE_TYPE(MAKE_MAPPING);
      #include "../node_type_x_macro.undef.h"
      #undef MAKE_MAPPING;
   }
}