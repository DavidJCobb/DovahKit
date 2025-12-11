#pragma once
#include "./node.h"

namespace dovah::datastores::impl::story_manager {
   class leaf_node : public node {
      public:
         using node::node;
   };
}