#pragma once
#include "helpers/const_forwarding_ptr.h"
#include "./idle_parent_node.h"
namespace dovah {
   class form_stub;
}
namespace dovah::datastores::impl::idles {
   class action_parent_node;
}

namespace dovah::datastores::impl::idles {
   class action_node : public idle_parent_node {
      public:
         constexpr action_node(datastore_type& d, form_stub& action) : idle_parent_node(d), stub(action) {}

      public:
         cobb::const_forwarding_ptr<action_parent_node> parent = nullptr;
         form_stub& stub;
   };
}