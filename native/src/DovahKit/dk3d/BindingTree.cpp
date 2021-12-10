#include "BindingTree.h"

namespace DK3D {
   void BindingTree::process() {
      assert(this->root);
      //
      auto*& active = this->active;
      if (!active)
         active = this->root;
      //
      auto& ih = DK3DInputHandler::get();
      //
      // Start with an upward pass, to verify that the active node is still active.
      //
      {
         binding_tree_nodes::base* highest_inactive = nullptr;
         for (auto* node = active; node; node = node->parent_node()) {
            if (node == this->root)
               break;
            if (!node->check_still_active(ih))
               highest_inactive = node;
         }
         if (highest_inactive) {
            active = highest_inactive->parent_node();
            assert(active);
         }
      }
      //
      // Now we do another upward pass: at each level in the active node and at each 
      // hierarchy level that we traverse, execute any active bindings UNLESS they 
      // are shadowed by a lower level's bindings.
      //
      for (auto* current = active; current; current = current->parent_node()) {
         for (auto* child : current->child_nodes()) {
            bool down = child->check_is_now_active(ih);
            if (!down)
               continue;
            bool is_parent = child->is_valid_parent();
            if (current == active) {
               if (is_parent)
                  active = child;
            } else {
               if (is_parent)
                  continue;
            }
            if (auto* casted = child->as<binding_tree_nodes::input>()) {
               if (auto* f = casted->tool)
                  f->invoke(result, casted->params, camera_update);
            }
         }
      }
   }
}