#include "./idle_parent_node.h"
#include "./idle_node.h"

namespace IdleAnimationFormsModel_impl {
   idle_parent_node::~idle_parent_node() {
      //
      // We need to define the constructor here so that its definition 
      // can access `idle_node::~idle_node` (for use by `std::unique_ptr`) 
      // without circular includes.
      //
   }
}