#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "helpers/const_forwarding_ptr.h"
#include "./anam_subrecord_position.h"
#include "./node_parent.h"
namespace dovah {
   namespace datastores {
      namespace impl::camera_paths::passkeys {
         class initial_build;
         class post_build_edit;
      }
      class camera_paths;
   }
   class form_stub;
}

namespace dovah::datastores::impl::camera_paths {
   class node : public node_parent {
      public:
         using datastore_type = ::dovah::datastores::camera_paths;

      protected:
         // Used during initial build. These are essentially IDLE/ANAM, but with 
         // node pointers instead of form-stub pointers, and with the same fixup 
         // that the game and CK do when they detect an invalid hierarchy.
         //
         // Cleared out after initial build.
         struct internal_sort_state {
            node_parent* parent   = nullptr;
            node*        previous = nullptr;
            bool         multiple_next_siblings = false;
         };

      public:
         constexpr node(datastore_type& d, form_stub& s) : datastore(d), stub(s) {};
         virtual ~node() {}

      public:
         datastore_type& datastore;
         form_stub&      stub;
         anam_subrecord_position first_seen_anam;
         cobb::const_forwarding_ptr<node_parent> parent = nullptr; // unowned
      protected:
         internal_sort_state _sort_state;

      public:
         bool is_defined_in_non_active_file() const noexcept;
         bool is_queued_for_processing_before(const node&) const noexcept;

         inline node* parent_as_node() noexcept {
            return dynamic_cast<node*>((node_parent*)this->parent);
         }
         inline const node* parent_as_node() const noexcept {
            return dynamic_cast<const node*>((const node_parent*)this->parent);
         }

      public: // passkeyed
         internal_sort_state& _get_sort_state(passkeys::initial_build) noexcept;
         void _update_form_hierarchy_data(passkeys::post_build_edit);
   };
}

#include "./node.inl"