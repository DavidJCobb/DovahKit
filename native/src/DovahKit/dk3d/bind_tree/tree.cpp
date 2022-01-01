#include "tree.h"
#include <algorithm>
#include "helpers/passkey.h"
#include "dk3d/DK3DInputHandler.h"
#include "node.h"
#include "nodes/editor_mode.h"
#include "nodes/input.h"
#include "nodes/root.h"

namespace DK3D::binds {
   tree::tree(input_device_type d) : device(d) {
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
   
   void tree::process(combined_tool_results& tap_results, combined_tool_results& while_results) {
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
         node* prior = active;
         node* after = nullptr;
         for (auto* node = active; node; node = node->parent_node()) {
            if (node == this->root)
               break;
            if (!node->check_still_active(ih))
               after = node;
         }
         if (after) {
            //
            // The active node or one of its ancestors has ceased to be active. We 
            // need to back out to the nearest still-active node, or to the root.
            //
            after  = after->parent_node();
            assert(after);
            active = after;
            //
            // A parent node has ceased to be active. Update states on any "while" 
            // button binds that were in that node; there are some edge-cases that 
            // we need to account for.
            // 
            // Consider the following binds:
            // 
            //    [L]     Modifier
            //    [U]     Test Shadowed
            //    [L + U] Test Shadower
            // 
            // If you press and hold [L], press and hold [U], release [L] and then 
            // release [U], then "Test Shadower" will stop running (because the 
            // parent modifier is no longer down) but will never receive a key-up 
            // "invoke" call, which is incorrect behavior.
            // 
            /// TODO: THE BELOW IS NOT FIXED; REQUIRES ACTION WHEN ENTERING A MODIFIER, NOT LEAVING ONE
            // 
            // If you press and hold [U], and then press and hold [L], then "Test 
            // Shadowed" will stop running (because it becomes shadowed), but it 
            // will never receive a key-up "invoke" call, which is also incorrect 
            // behavior.
            // 
            /// TODO: THE ABOVE IS NOT FIXED; REQUIRES ACTION WHEN ENTERING A MODIFIER, NOT LEAVING ONE
            // 
            // Lastly, if you press and hold [L] and [U] on the same frame, and 
            // then release both keys on the same frame, then "Test Shadower" will 
            // stop running but will never receive a key-up "invoke" call, because 
            // (if we don't do any special processing here) we'll have backed out 
            // of the modifier; "Test Shadowed" will incorrectly receive the key-up 
            // "invoke" call.
            // 
            // These are the cases we want to fix here. When we back out of any 
            // parent nodes, we need to check the "while" button binds inside of 
            // those nodes, searching from the deepest nodes to the shallowest. We 
            // need to fake "key-up" events in the first two cases; in the third 
            // case, we need to send an authentic "key-up" event here and then 
            // make sure that that event is not also received by any shadowed bind.
            //
            for (const auto* node = prior; node && node != after; node = node->parent_node()) {
               for (const auto* child : node->child_nodes()) {
                  auto* input = child->as<nodes::input>();
                  if (!input)
                     continue;
                  auto& mapping = input->mapping;
                  auto& button  = mapping.button;
                  if (button.press_type != button_press_type::while_down)
                     continue;
                  if (!mapping.is_button())
                     continue;
                  if (ih._isButtonProcessed(ih_passkey_t(), button))
                     //
                     // If we've already passed a key-up for this key, don't pass 
                     // another.
                     //
                     continue;
                  //
                  auto ir = ih.inputResultOf(mapping);
                  if (ir.active()) {
                     //
                     // The key is currently down, or just went down. Send a faked key-up 
                     // to this binding. Don't mark the binding as processed; we want any 
                     // (formerly) shadowed binds to the same key to be able to detect the 
                     // key-down on this frame.
                     //
                     InputResult faked;
                     faked.while_has_changed = true;
                     if (input->tool)
                        input->tool->invoke(faked, input->params, while_results);
                  } else if (ir.while_has_changed) {
                     //
                     // The key just went up. Send a real key-up to this binding, and then 
                     // mark the key as processed so that (formerly) shadowed "while" binds 
                     // in ancestor nodes  don't also receive the same key-up.
                     //
                     if (input->tool)
                        input->tool->invoke(ir, input->params, while_results);
                     ih._markButtonProcessed(ih_passkey_t(), mapping.button);
                  }
               }
            }
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
                  if (casted->tool) {
                     bool is_tap = (r.type == control_type::button && r.press_type != button_press_type::while_down);
                     casted->tool->invoke(r, casted->params, is_tap ? tap_results : while_results);
                  }
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