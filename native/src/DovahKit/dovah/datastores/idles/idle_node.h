#pragma once
#include "helpers/const_forwarding_ptr.h"
#include "./idle_parent_node.h"
#include "./passkeys/idle_sorting.h"
namespace dovah {
   class form_stub;
}

namespace dovah::datastores::impl::idles {
   class idle_node : public idle_parent_node {
      public:
         constexpr idle_node(datastore_type& d, form_stub& idle) : idle_parent_node(d), stub(idle) {}

      public:
         cobb::const_forwarding_ptr<idle_parent_node> parent = nullptr;
         form_stub& stub;
      protected:
         struct {
            idle_parent_node* parent_idle   = nullptr;
            idle_node*        previous_idle = nullptr;
         } sort_state;

      public:
         constexpr auto& _get_sort_state(passkeys::idle_sorting) noexcept { return this->sort_state; }
   };
}