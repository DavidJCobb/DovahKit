#include "tree.h"
#include <algorithm>
#include "helpers/passkey.h"
#include "dk3d/DK3DInputHandler.h"
#include "node.h"
#include "nodes/editor_mode.h"
#include "nodes/input.h"
#include "nodes/root.h"

namespace DK3D::binds {
   tree::tree(InputDevice d) : device(d) {
      this->root = new nodes::root;
   }
   
   tree::tree(const tree& o) {
      this->root = (nodes::root*)o.root->clone();
   }
   tree& tree::operator=(const tree& o) {
      if (this->root)
         delete this->root;
      this->device = o.device;
      this->active = nullptr;
      this->root   = (nodes::root*)o.root->clone();
      return *this;
   }
   
   tree::tree(tree&& o) {
      std::swap(this->device, o.device);
      std::swap(this->active, o.active);
      std::swap(this->root,   o.root);
   }
   tree& tree::operator=(tree&& o) {
      std::swap(this->device, o.device);
      std::swap(this->active, o.active);
      std::swap(this->root,   o.root);
      return *this;
   }
   
   void tree::process(DKVulkanCameraUpdate& camera_update) {
      assert(this->root);
      //
      auto*& active = this->active;
      if (!active)
         active = this->root;
      //
      auto& ih = DK3DInputHandler::get();
      using ih_passkey_t = cobb::passkey<DK3DInputHandler, tree>;
      //
      // Start with an upward pass, to verify that the active node is still active.
      //
      if (active != this->root) {
         node* highest_inactive = nullptr;
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
      node* original_active = active;
      node* previous_node   = nullptr;
      for (auto* current = active; current; previous_node = current, current = current->parent_node()) {
         if (auto* casted = current->as<nodes::input>()) {
            //
            // For modifier key nodes, mark their keys as processed, so that the 
            // modifiers themselves "shadow" nodes earlier in the tree.
            // 
            // NOTE: This only works if we have exactly one tree per input device, 
            // as we only clear the "processed" flags when we update the input 
            // device state once per frame.
            //
            auto& bi = casted->mapping;
            if (bi.is_button()) {
               ih._markButtonProcessed(ih_passkey_t(), bi.button);
            }
         }
         for (auto* child : current->child_nodes()) {
            if (child == previous_node) // when traversing up the tree and executing children at each level, don't execute the nodes we've traversed from
               continue;
            //
            // Each node type requires special handling.
            //
            if (auto* casted = child->as<nodes::input>()) {
               auto& mapping = casted->mapping;
               if (mapping.is_button()) {
                  //
                  // Buttons deeper in the tree should shadow buttons shallower in 
                  // the tree.
                  //
                  if (ih._isButtonProcessed(ih_passkey_t(), mapping.button))
                     continue;
                  ih._markButtonProcessed(ih_passkey_t(), mapping.button);
               }
               auto r = ih.inputResultOf(mapping);
               bool a = r.active();
               if (a || r.while_has_changed) {
                  if (casted->tool)
                     casted->tool->invoke(r, casted->params, camera_update);
                  //
                  // Handle modifier keys:
                  //
                  if (current == original_active && a) {
                     if (child->is_valid_parent())
                        active = child;
                  }
               }
               continue;
            }
            if (auto* casted = child->as<nodes::editor_mode>()) {
               //
               // TODO: If the editor has switched to this mode, and if this node is a 
               // direct child of the original active node, then set this node as the 
               // active node.
               //
               continue;
            }
            //
            // Handle any new node types here.
            //
         }
      }
   }
}