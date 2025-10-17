#pragma once
#include <memory>
#include <vector>
#include "./_unique_ptr_utils.h"
namespace dovah {
   class form_stub;
}

namespace IdleAnimationFormsModel_impl {
   namespace impl {
      template<typename Node>
      concept node_for_form = requires (Node& node) {
         { node.stub } -> std::same_as<dovah::form_stub*&>;
      };
   };

   template<typename ChildNode>
   class _parent_node_mixin {
      public:
         using child_node_type = ChildNode;

         static constexpr const size_t no_index = (size_t)-1;

      public:
         std::vector<node_unique_ptr<child_node_type>> children;

      public:
         constexpr size_t index_of_child(const child_node_type&) const noexcept;
         constexpr size_t index_of_form(dovah::form_stub&) const noexcept requires impl::node_for_form<child_node_type>;

         constexpr const child_node_type* child_by_form(dovah::form_stub& stub) const noexcept requires impl::node_for_form<child_node_type> {
            auto idx = index_of_form(stub);
            return (idx == no_index) ? nullptr : this->children[idx].get();
         }
         constexpr child_node_type* child_by_form(dovah::form_stub& stub) noexcept requires impl::node_for_form<child_node_type> {
            return const_cast<child_node_type*>(std::as_const(*this).child_by_form(stub));
         }

         constexpr node_unique_ptr<child_node_type> take_child(size_t);
   };
}

#include "./_parent_node_mixin.inl"