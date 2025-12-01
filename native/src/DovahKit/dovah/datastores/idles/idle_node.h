#pragma once
#include "helpers/const_forwarding_ptr.h"
#include "./idle_parent_node.h"
#include "./passkeys/idle_sorting.h"
#include "./passkeys/push_idle_hierarchy_position_to_form.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class action_node;
   namespace passkeys {
      class action_update_root;
      class fully_delete_action;
   }
}

namespace dovah::datastores::impl::idles {
   class idle_node : public idle_parent_node {
      public:
         constexpr idle_node(datastore_type& d, form_stub& idle) : idle_parent_node(d), stub(idle) {}
         ~idle_node();

      public:
         cobb::const_forwarding_ptr<idle_parent_node> parent = nullptr;
         form_stub& stub;
      protected:
         struct {
            idle_parent_node* parent_idle   = nullptr;
            idle_node*        previous_idle = nullptr;
         } sort_state;
         struct {
            std::vector<action_node*> masters;
            std::vector<action_node*> active;
         } action_root_candidacies; // mainly tracked so that if this idle is deleted, we can notify whatever action it is the root of

      public:
         constexpr auto& _get_sort_state(passkeys::idle_sorting) noexcept { return this->sort_state; }

         // Returns the parent action, if there is no parent idle or LOOSE node.
         action_node* get_parent_action() const noexcept;

      public: // passkeyed
         void _build_action_root_candidacy(passkeys::initial_build_action_root, action_node&, bool is_active_file);

         void _on_action_fully_deleted(passkeys::fully_delete_action, action_node&);

         void _clear_active_action_root_candidacies(passkeys::action_update_root, action_node&);
         void _add_active_action_root_candidacy(passkeys::action_update_root, action_node&);

         // Update the IDLE form's ANAM subrecord (indicating its parent and previous sibling), 
         // while also triggering the owning datastore's form-changed callbacks.
         void _on_hierarchy_changed(passkeys::push_idle_hierarchy_position_to_form, size_t my_new_index = no_index);

         // TODO: Abandon active-file action root candidacies, with whatever knock-on effects 
         // that would have.
         void _on_become_child_of_idle();
   };
}