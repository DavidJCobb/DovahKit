#pragma once
#include <memory>
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./node.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class idle_node;
   class warning;
}
namespace dovah::datastores::impl::idles::passkeys {
   class idle_sorting;
}

namespace dovah::datastores::impl::idles {
   class idle_parent_node : public node {
      public:
         ~idle_parent_node();

      public:
         std::vector<cobb::const_forwarding_ptr<idle_node>> children; // owned

      public:
         // The to-be-appended node must not have a parent.
         void append_child(std::unique_ptr<idle_node>&&);

         // If the to-be-appended node is already a child of this node, this is a no-op.
         // If the to-be-appended node is the child of some other node, it will be removed 
         // from that node.
         void append_child(idle_node&);

         void destroy_child(size_t);
         constexpr size_t index_of_child(const idle_node&) const noexcept;
         size_t index_of_child(const form_stub&) const noexcept;
         void insert_child_at(idle_node&, size_t);
         std::unique_ptr<idle_node> take_child(size_t);

         void sort_children();
         void sort_descendants();

         void insert_sorted_child(passkeys::idle_sorting, idle_node&, std::vector<warning*>&);
   };
}

#include "./idle_parent_node.inl"