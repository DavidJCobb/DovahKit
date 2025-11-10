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
         using node::node;
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
         void insert_child_before(idle_node&, size_t);
         void insert_child_after(idle_node&, size_t);
         std::unique_ptr<idle_node> take_child(size_t);

         // Used by initial build when compile-time configuration is set to sort idles after 
         // all idle trees are built.
         void sort_children(passkeys::idle_sorting);
         void sort_descendants(passkeys::idle_sorting);

         // Used by initial build when compile-time configuration is set to sort idles as the 
         // idle trees are being built.
         void insert_sorted_child(passkeys::idle_sorting, idle_node&, std::vector<warning*>&);

      protected:
         void _move_child_to_index(size_t from, size_t to);
         void _adopt_new_child_to_index(idle_node&, size_t at);

         // Used only after initial build, to nodify an idle that its position in the hierarchy 
         // has been changed. The idle will push its new position to the loaded form data.
         void _on_previous_sibling_changed(size_t subject_index);
         void _on_previous_sibling_changed(idle_node& subject);
         void _on_parent_changed(idle_node& subject, size_t subject_index = no_index);
   };
}

#include "./idle_parent_node.inl"