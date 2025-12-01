#pragma once
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./action_root_candidacy.h"
#include "./node.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class graph_node;
   class idle_node;
   namespace passkeys {
      class initial_build;
      class post_build_edit;
   }
}

namespace dovah::datastores::impl::idles {
   class action_node : public node {
      public:
         action_node(datastore_type& d, form_stub&);

         struct candidacy : public action_root_candidacy {
            idle_node* idle = nullptr;
         };

      public:
         form_stub&  stub;
         graph_node* graph = nullptr; // unowned
         cobb::const_forwarding_ptr<idle_node> winning_root; // unowned
         struct {
            //
            // These lists are kept sorted, with the most recently loaded candidacy 
            // at the end.
            //
            std::vector<candidacy> masters;
            std::vector<candidacy> active;
         } candidacies;

      public:
         bool candidates_include(const idle_node&) const noexcept;

      public: // passkeyed
         // initial-build:
         void _track_candidate(passkeys::initial_build, const action_root_candidacy&, idle_node&, bool via_master);

         // post-build:
         void _untrack_active_file_candidate(passkeys::post_build_edit, idle_node&); // caller must update the winning-root afterward
         void _track_active_file_candidate(passkeys::post_build_edit, idle_node&);
         void _recalc_winning_root(passkeys::post_build_edit);
   };
}