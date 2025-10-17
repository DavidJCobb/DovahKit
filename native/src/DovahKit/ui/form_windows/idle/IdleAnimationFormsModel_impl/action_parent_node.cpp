#include "./action_parent_node.h"
#include "./action_node.h"

namespace IdleAnimationFormsModel_impl {
   action_parent_node::~action_parent_node() {
      //
      // We need to define the constructor here so that its definition can 
      // access `action_node::~action_node` (for use by `std::unique_ptr`) 
      // without circular includes.
      //
   }
}