#include "tree.h"
#include <algorithm>
#include "helpers/passkey.h"
#include "../worldinput2.h"
#include "./node.h"
#include "./nodes/bound_tool.h"
#include "./nodes/editor_mode.h"
#include "./nodes/modifier.h"
#include "./nodes/root.h"
#include "./nodes/abstract_input_node.h"

namespace dovahkit::subsystems::worldinput2::binds {
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
      this->root   = (nodes::root*)o.root->clone();
      return *this;
   }
   
   tree::tree(tree&& o) {
      std::swap(this->device, o.device);
      std::swap(this->root,   o.root);
   }
   tree& tree::operator=(tree&& o) {
      std::swap(this->device, o.device);
      std::swap(this->root,   o.root);
      return *this;
   }
   
   void tree::process(combined_tool_results& tap_results, combined_tool_results& while_results) {
      assert(this->root);

      std::vector<node*> eligible_binds;
      std::vector<nodes::abstract_input_node*> conflict_losing_binds;

      // Traversal:
      for (auto* child : this->root->child_nodes()) {
         nodes::abstract_input_node* input_node = nullptr;

         bool matched = false;
         switch (child->type) {
            case node_type::editor_mode:
               matched = static_assert(false, "TODO: True if Worldedit says we're in the node's desired editing mode.");
               break;
            case node_type::bound_tool:
            case node_type::modifier:
               {
                  input_node = (nodes::abstract_input_node*)child;

                  auto& sequence = input_node->input_sequence;

                  sequence.update(static_assert(false, "TODO: pass the current timestamp"));
                  switch (input_node->button_press_type) {
                     case button_press_type::hold:
                        matched = sequence.state.frame_status == input_sequence::frame_status::down;
                        break;
                     default:
                        matched = sequence.state.frame_status == input_sequence::frame_status::released;
                        break;
                  }
               }
               break;
         }
         //
         if (matched) {
            if (input_node) {
               bool lost_conflict = false;
               for (auto* loser : conflict_losing_binds) {
                  if (loser->button_press_type != input_node->button_press_type)
                     continue;
                  if (loser->absolute_terminal_inputs() != input_node->absolute_terminal_inputs())
                     continue;
                  lost_conflict = true;
                  break;
               }
               if (lost_conflict) {
                  conflict_losing_binds.push_back(input_node);
                  continue;
               }
               //
               bool child_conflicted = false;
               {
                  std::vector<nodes::abstract_input_node*> binds_made_ineligible;
                  for (auto* prior_bind : eligible_binds) {
                     switch (prior_bind->type) {
                        case node_type::bound_tool:
                        case node_type::modifier:
                           break;
                        default:
                           continue;
                     }

                     nodes::abstract_input_node* winning_node;
                     bool winner_is_not_blocked;
                     static_assert(false, "TODO: node conflict resolution algorithm, passing refs to the above two variables");

                     auto* losing_node = (winning_node == input_node) ? (nodes::abstract_input_node*)prior_bind : input_node;

                     if (losing_node->button_press_type != button_press_type::hold) {
                        conflict_losing_binds.push_back(losing_node);
                     }
                     if (winning_node == input_node) {
                        binds_made_ineligible.push_back(losing_node);
                        if (!winner_is_not_blocked) {
                           child_conflicted = true;
                        }
                     } else {
                        child_conflicted = true;
                     }
                  }
                  for (auto* bind : binds_made_ineligible) {
                     eligible_binds.erase(
                        std::remove_if(
                           eligible_binds.begin(),
                           eligible_binds.end(),
                           [bind](auto* node) {
                              return node == bind;
                           }
                        ),
                        eligible_binds.end()
                     );
                  }
               }
               //
               if (child_conflicted) {
                  if (input_node->button_press_type != button_press_type::hold)
                     conflict_losing_binds.push_back(input_node);
                  continue;
               }
               eligible_binds.push_back(input_node);
            }
            for (auto* item : child->child_nodes()) {
               static_assert(false, "TODO: recurse");
            }
         } else {
            // Not matched.
            input_node->clear_descendants_progress();
         }
      }
      
      for (auto* conflicted : conflict_losing_binds) {
         if (conflicted->button_press_type == button_press_type::hold)
            continue;
         conflicted->clear_descendants_progress();
      }

      // Hold release.
      for (auto* hold_node : this->last_frame_active_hold_binds) {
         assert(hold_node->type == node_type::bound_tool);
         if (std::find(eligible_binds.begin(), eligible_binds.end(), hold_node) == eligible_binds.end()) {
            static_assert(false, "TODO: Execute the bound tool in the context of key-up.");
         }
      }
      this->last_frame_active_hold_binds = {};

      // Execution;
      for (auto* node : eligible_binds) {
         if (auto* bt = node->as<nodes::bound_tool>()) {
            static_assert(false, "TODO: Execute the bound tool.");
            if (bt->button_press_type == button_press_type::hold)
               this->last_frame_active_hold_binds.push_back(bt);
         }
         if (auto* in = node->as<nodes::abstract_input_node>()) {
            if (in->button_press_type != button_press_type::hold) {
               assert(in->input_sequence.state.frame_status == input_sequence::frame_status::released);
               static_assert(false, "TODO: Assert that all groups inside of the input sequence have a frame status of 'inactive'.");
               in->input_sequence.state.frame_status = input_sequence::frame_status::inactive;
            }
         }
      }
   }
}