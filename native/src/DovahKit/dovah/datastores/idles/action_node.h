#pragma once
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./action_root_candidacy.h"
#include "./node.h"
namespace dovah {
   class file_load_order;
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class action_parent_node;
   class idle_mirror_node;
   class idle_node;
   namespace passkeys {
      class fully_delete_idle;
      class initial_build_action_root;
   }
}

namespace dovah::datastores::impl::idles {
   class action_node : public node {
      public:
         constexpr action_node(datastore_type& d, form_stub& action) : node(d), stub(action) {}
         ~action_node();

      protected:
         struct tracked_candidacy : public action_root_candidacy {
            idle_node* candidate = nullptr;
         };

      public:
         cobb::const_forwarding_ptr<action_parent_node> parent = nullptr;
         form_stub& stub;
      protected:
         struct {
            std::vector<tracked_candidacy> masters;
            std::vector<tracked_candidacy> active;
         } action_root_candidacies;

      public:
         constexpr idle_node* get_winning_root_idle() noexcept;
         constexpr const idle_node* get_winning_root_idle() const noexcept;

         void set_active_root(idle_node&);
         void unset_active_root(idle_node&);

      public: // passkeyed
         void _register_root_idle(passkeys::initial_build_action_root, const file_load_order&, idle_node&, const action_root_candidacy&);

         void _on_idle_fully_deleted(passkeys::fully_delete_idle, idle_node&);
   };
}

#include "./action_node.inl"