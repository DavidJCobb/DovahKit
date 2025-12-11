#pragma once
#include "helpers/const_forwarding_ptr.h"
namespace dovah {
   namespace datastores {
      namespace impl::story_manager {
         namespace passkeys {
            class post_build_edit;
         }
         class branch_node;
      }
      class story_manager;
   }
   class form_stub;
}

namespace dovah::datastores::impl::story_manager {
   class node {
      public:
         using datastore_type = ::dovah::datastores::story_manager;

         struct build_state {
            bool previous_intentionally_null = false;
            bool previous_ended_up_null      = false;
         };

      public:
         constexpr node(datastore_type& d, form_stub& s) : datastore(d), stub(s) {};
         virtual ~node() {}

      public:
         datastore_type& datastore;
         form_stub&      stub;
         cobb::const_forwarding_ptr<branch_node> parent = nullptr;
      protected:
         build_state _build_state;

      public: // passkeyed
         constexpr build_state& _get_build_state(passkeys::initial_build) noexcept { return this->_build_state; }
         void _update_form_hierarchy_data(passkeys::post_build_edit);
   };
}