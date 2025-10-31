#include "./graph_node.h"
#include "./action_node.h"
#include "../../form_stub.h"
#include "../../form_types.h"

namespace dovah::datastores::impl::idles {
   graph_node::graph_node() {
      this->loose = new idle_parent_node;
   }
   graph_node::~graph_node() {
      if (auto*& ptr = this->loose) {
         delete ptr;
         ptr = nullptr;
      }
   }
}
