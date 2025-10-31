#pragma once
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
         constexpr action_node(form_stub& action) : stub(action) {}

      public:
         action_parent_node* parent = nullptr;
         form_stub&          stub;
   };
}