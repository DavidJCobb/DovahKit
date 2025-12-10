#pragma once
#include <vector>
#include "helpers/const_forwarding_ptr.h"
namespace dovah {
   namespace datastores {
      namespace impl::camera_paths {
         namespace passkeys {
            class initial_build;
            class post_build_edit;
         }
         class node;
      }
      class camera_paths;
   }
   class form_stub;
}


namespace dovah::datastores::impl::camera_paths {
   class node_parent {
      public:
         static constexpr const size_t index_of_none = -1;

         virtual ~node_parent() {}

      public:
         std::vector<cobb::const_forwarding_ptr<node>> children; // unowned

      public:
         constexpr size_t index_of_child(const node&) const noexcept;
         size_t index_of_child(const form_stub&) const noexcept;
         bool is_ancestor_of(const node&) const noexcept;

      public: // passkeyed
         void _insert_sorted_child(passkeys::initial_build, node&);
   };
}

#include "./node_parent.inl"