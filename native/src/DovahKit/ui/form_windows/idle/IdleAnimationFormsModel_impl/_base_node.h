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
         virtual size_t index_of_child(const node&) const = 0;
         virtual const node* nth_child(size_t) const = 0;
         virtual void update_cached_form_data() = 0;

         const node_type type;
         node* parent = nullptr;

         node* nth_child(size_t n) {
            return const_cast<node*>(std::as_const(*this).nth_child(n));
         }

         template<node_type Enum>
         const utils::node_type_from_enum<Enum>* as() const noexcept;
         template<node_type Enum>
         utils::node_type_from_enum<Enum>* as() noexcept;
   };
}

#include "./_base_node.inl"
