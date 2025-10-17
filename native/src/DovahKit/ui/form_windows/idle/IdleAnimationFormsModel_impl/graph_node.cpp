#include "./graph_node.h"
#include "./idle_parent_node.h"
#include "./idle_node.h"

namespace IdleAnimationFormsModel_impl {
   graph_node::graph_node() : action_parent_node(node_type::graph) {
   }
   graph_node::~graph_node() {
   }

   /*virtual*/ void graph_node::update_cached_form_data() /*override*/ {
   }

   void graph_node::insert_loose_idle(node_unique_ptr<idle_node>&& node_ptr) {
      if (!node_ptr)
         return;
      auto& parent_ptr = this->loose.idles;
      if (!parent_ptr) {
         parent_ptr = make_unique<loose_idle_parent_node>();
         parent_ptr->owner = this;
      }
      parent_ptr->children.push_back(std::move(node_ptr));
   }
}