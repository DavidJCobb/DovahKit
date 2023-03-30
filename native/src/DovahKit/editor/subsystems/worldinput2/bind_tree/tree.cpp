#include "tree.h"
#include <algorithm>
#include "helpers/passkey.h"
#include "../devices/abstract_device_handler.h"
#include "../worldinput2.h"
#include "./node.h"
#include "./nodes/bound_tool.h"
#include "./nodes/editor_mode.h"
#include "./nodes/modifier.h"
#include "./nodes/root.h"
#include "./nodes/abstract_input_node.h"

#include "editor/subsystems/worldedit/core.h"

namespace dovahkit::subsystems::worldinput2::binds {
   tree::tree(input_device_type d) : device_type(d) {
      this->root = new nodes::root;
   }
   
   tree::tree(const tree& o) {
      this->root = (nodes::root*)o.root->clone();
   }
   tree& tree::operator=(const tree& o) {
      if (this->root)
         delete this->root;
      this->device_type = o.device_type;
      this->root        = (nodes::root*)o.root->clone();
      return *this;
   }
   
   tree::tree(tree&& o) {
      std::swap(this->device_type, o.device_type);
      std::swap(this->root,        o.root);
   }
   tree& tree::operator=(tree&& o) {
      std::swap(this->device_type, o.device_type);
      std::swap(this->root,        o.root);
      return *this;
   }

   tree::~tree() {
      if (this->root) {
         delete this->root;
         this->root = nullptr;
      }
   }
   
   void tree::update(timestamp_t now, combined_tool_results& tap_results, combined_tool_results& while_results) {
      assert(this->root);

      auto& subsys = core::get(); // worldinput2
      devices::abstract_device_handler& device = subsys.device_by_type(this->device_type);

      auto current_editing_mode = worldedit::core::get().get_editor_mode();

      std::vector<node*> eligible_binds;
      std::vector<nodes::abstract_input_node*> conflict_losing_binds;

      auto traversal_subalgorithm = [&eligible_binds, &conflict_losing_binds, &device, current_editing_mode](node* current) {
         auto recurse = [&](node* current, auto& recurse) mutable -> void {
            //
            for (auto* child : current->child_nodes()) {
               nodes::abstract_input_node* child_inode = nullptr;

               bool matched = false;
               switch (child->type) {
                  case node_type::editor_mode:
                     matched = ((nodes::editor_mode*)child)->mode == current_editing_mode;
                     break;
                  case node_type::bound_tool:
                  case node_type::modifier:
                     {
                        child_inode = (nodes::abstract_input_node*)child;

                        auto& sequence = child_inode->input_sequence;

                        sequence.update(now, device);
                        switch (child_inode->button_press_type) {
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
               if (!matched) {
                  if (child_inode)
                     child_inode->clear_descendants_progress();
                  continue;
               }
               if (child_inode) {
                  bool lost_conflict = false;
                  for (auto* loser : conflict_losing_binds) {
                     if (loser->button_press_type != child_inode->button_press_type)
                        continue;
                     if (loser->absolute_terminal_inputs() != child_inode->absolute_terminal_inputs())
                        continue;
                     lost_conflict = true;
                     break;
                  }
                  if (lost_conflict) {
                     conflict_losing_binds.push_back(child_inode);
                     continue;
                  }
                  //
                  bool child_conflicted = false;
                  {
                     std::vector<nodes::abstract_input_node*> binds_made_ineligible;
                     for (auto* prior : eligible_binds) {
                        auto* prior_inode = prior->as<nodes::abstract_input_node>();
                        if (!prior_inode)
                           continue;

                        nodes::abstract_input_node* winning_node;
                        bool winner_is_not_blocked;
                        static_assert(false, "TODO: node conflict resolution algorithm, passing refs to the above two variables");

                        auto* losing_node = (winning_node == child_inode) ? prior_inode : child_inode;

                        if (losing_node->button_press_type != button_press_type::hold) {
                           conflict_losing_binds.push_back(losing_node);
                        }
                        if (winning_node == child_inode) {
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
                     if (child_inode->button_press_type != button_press_type::hold)
                        conflict_losing_binds.push_back(child_inode);
                     continue;
                  }
                  eligible_binds.push_back(child_inode);
               }
               for (auto* item : child->child_nodes()) {
                  recurse(item, recurse);
               }
            }
            //
         };
         recurse(current, recurse);
      };
      //
      traversal_subalgorithm(this->root);
      
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
               assert(in->input_sequence.all_contents_inactive());
               in->input_sequence.state.frame_status = input_sequence::frame_status::inactive;
            }
         }
      }
   }
}