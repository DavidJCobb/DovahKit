#pragma once
#include <memory>
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./node.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class action_node;
}

namespace dovah::datastores::impl::idles {
   class action_parent_node : public node {
      public:
         using node::node;
         ~action_parent_node();

      public:
         std::vector<cobb::const_forwarding_ptr<action_node>> children; // owned

      public:
         static bool sort_comparator(const action_node*, const action_node*);

         // The to-be-appended node must not have a parent.
         void append_child(std::unique_ptr<action_node>&&);

         // If the to-be-appended node is already a child of this node, this is a no-op.
         // If the to-be-appended node is the child of some other node, it will be removed 
         // from that node.
         void append_child(action_node&);

         void destroy_child(size_t);
         constexpr size_t index_of_child(const action_node&) const noexcept;
         size_t index_of_child(const form_stub&) const noexcept;
         std::unique_ptr<action_node> take_child(size_t);

         void sort_children();
         [[nodiscard]] std::vector<size_t> sort_children_and_remember();
         void sort_descendants();

         const action_node* action_by_stub(const form_stub&) const noexcept;
         action_node* action_by_stub(const form_stub&) noexcept;

         action_node* get_or_create_action(form_stub&);
   };
}

#include "./action_parent_node.inl"