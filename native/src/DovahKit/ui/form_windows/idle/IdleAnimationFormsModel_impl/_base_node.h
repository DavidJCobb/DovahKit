#pragma once
#include "./node_type.h"
#include "./utils/node_type_from_enum.h"

namespace IdleAnimationFormsModel_impl {
   class node {
      public:
         static constexpr const size_t index_of_none = (size_t)-1;

      protected:
         constexpr node(node_type t) : type(t) {}
      public:
         virtual ~node() {}
         virtual void update_cached_form_data() {}

         const node_type type;

         template<node_type Enum>
         const utils::node_type_from_enum<Enum>* as() const noexcept;

         template<node_type Enum>
         utils::node_type_from_enum<Enum>* as() noexcept;
   };
}

#include "./_base_node.inl"
