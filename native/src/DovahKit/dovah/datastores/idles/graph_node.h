#pragma once
#include <string>
#include "./action_parent_node.h"
#include "./idle_parent_node.h"
namespace dovah {
   class form_stub;
}

namespace dovah::datastores::impl::idles {
   class graph_node : public action_parent_node {
      public:
         graph_node();
         ~graph_node();

      public:
         idle_parent_node* loose = nullptr; // owned
         std::string path;
   };
}