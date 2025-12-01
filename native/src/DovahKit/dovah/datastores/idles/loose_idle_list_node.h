#pragma once
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./node.h"
namespace dovah::datastores::impl::idles {
   class graph_node;
   class idle_node;
   namespace passkeys {
      class post_build_edit;
   }
}

namespace dovah::datastores::impl::idles {
   class loose_idle_list_node : public node {
      public:
         using node::node;

      public:
         graph_node* graph = nullptr; // unowned
         std::vector<cobb::const_forwarding_ptr<idle_node>> child_idles; // unowned

      public:
         static bool idle_sort_comparator(const idle_node*, const idle_node*);

      public:
         size_t index_of_child(const idle_node&) const noexcept;

         // What index would a node be placed at, were it to be inserted?
         size_t prospective_index_of(const idle_node&) const noexcept;

         void re_sort_idle(passkeys::post_build_edit, size_t);
   };
}