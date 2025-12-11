#pragma once
#include <vector>
#include "./node.h"
namespace dovah::datastores::impl::story_manager::passkeys {
   class initial_build;
   class post_build_edit;
}

namespace dovah::datastores::impl::story_manager {
   class branch_node : public node {
      public:
         static constexpr const size_t index_of_none = (size_t)-1;

         using node::node;

      public:
         std::vector<cobb::const_forwarding_ptr<node>> children;

      public:
         constexpr bool is_ancestor_of(const node&) const noexcept;
         constexpr size_t index_of_child(const form_stub&) const noexcept;
         constexpr size_t index_of_child(const node&) const noexcept;

      public: // passkey
         void _append_during_load(passkeys::initial_build, node&);
         void _insert_during_load(passkeys::initial_build, node&, size_t at);
         void _insert_during_load(passkeys::initial_build, node&, const node& after);
   };
}

#include "./branch_node.inl"