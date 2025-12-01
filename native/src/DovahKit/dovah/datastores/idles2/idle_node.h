#pragma once
#include <string_view>
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./action_root_candidacy.h"
#include "./node.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles2 {
   class action_node;
   class graph_node;
   namespace passkeys {
      class initial_build;
      class post_build_edit;
   }
}

namespace dovah::datastores::impl::idles2 {
   class idle_node : public node {
      public:
         idle_node(datastore_type& d, form_stub&);

         // Used during initial build. These are essentially IDLE/ANAM, but with 
         // node pointers instead of form-stub pointers, and with the same fixup 
         // that the game and CK do when they detect an invalid hierarchy.
         //
         // Cleared out after initial build.
         struct internal_sort_state {
            idle_node* parent_idle   = nullptr;
            idle_node* previous_idle = nullptr;
         };

         struct candidacy : public action_root_candidacy {
            action_node* action = nullptr; // unowned
         };

      public:
         form_stub& stub;
         //
         node* canonical_parent = nullptr; // action_node, idle_node, or loose_idle_list_node // unowned
         std::vector<cobb::const_forwarding_ptr<idle_node>> child_idles; // unowned
         struct {
            std::vector<candidacy> masters;
            std::vector<candidacy> active;
         } is_candidate_for;
         internal_sort_state _sort_state;

      public:
         std::string canonical_graph_path() const noexcept; // retrieved via the form stub
         bool is_defined_in_non_active_file() const noexcept;
         bool is_forced_loose() const noexcept; // retrieved via the form stub

         const graph_node* containing_graph() const noexcept; // retrieved by walking up the canonical parents
         graph_node* containing_graph() noexcept;

         bool contains(const idle_node&) const noexcept;
         size_t index_of_child(const idle_node&) const noexcept;

         bool is_active_candidate_for(const action_node&) const noexcept;
         bool is_winning_root_of_action_in_own_graph() const noexcept;

         // initial build:
         internal_sort_state& _get_sort_state(passkeys::initial_build) noexcept;
         void _track_loaded_candidacy(passkeys::initial_build, action_node&, const action_root_candidacy&, bool is_master);
         void _insert_sorted_child(passkeys::initial_build, idle_node&);

         // post-build:
         void _track_new_active_candidacy(passkeys::post_build_edit, action_node&);
         void _update_form_hierarchy_data(passkeys::post_build_edit);
   };
}