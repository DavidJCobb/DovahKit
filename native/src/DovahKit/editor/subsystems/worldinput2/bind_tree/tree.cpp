#include "tree.h"
#include <algorithm>
#include "helpers/passkey.h"
#include "../devices/abstract_device_handler.h"
#include "../tools/combined_tool_results.h"
#include "../core.h"
#include "../defaults.h"
#include "../interruption_check.h"
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
      assert(o.root);
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
   
   void tree::update(timestamp_t now, combined_tool_results& press_results, combined_tool_results& hold_results) {
      assert(this->root);

      auto& subsys = core::get(); // worldinput2
      devices::abstract_device_handler& device = subsys.device_by_type(this->device_type);

      auto current_editing_mode = worldedit::core::get().get_editor_mode();

      std::vector<nodes::abstract_input_node*> eligible_binds;
      std::vector<nodes::abstract_input_node*> conflict_losing_binds;

      interruption_check interruption_check = device.prepare_interruption_check();

      auto traversal_subalgorithm = [&eligible_binds, &conflict_losing_binds, &device, &interruption_check, current_editing_mode, now](node* current) {
         auto recurse = [&](node* current, auto& recurse) mutable -> void {
            //
            for (auto* child : current->child_nodes()) {
               nodes::abstract_input_node* child_inode = nullptr;

               assert(child != nullptr);

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

                        sequence.update(now, device, interruption_check);
                        if (sequence.state.frame_status == input_sequence::frame_status::released) {
                           switch (child_inode->button_press_type) {
                              case button_press_type::press:
                                 matched = true;
                                 break;
                              case button_press_type::long_press:
                                 {
                                    auto down_for = elapsed_time(child_inode->input_sequence.state.went_down_at, now);
                                    if (down_for >= defaults::press_to_long_press_threshold)
                                       matched = true;
                                 }
                                 break;
                           }
                        } else if (sequence.state.frame_status == input_sequence::frame_status::down) {
                           switch (child_inode->button_press_type) {
                              case button_press_type::hold:
                                 matched = true;
                                 break;
                           }
                        }
                     }
                     break;
               }
               //
               if (!matched) {
                  if (child_inode)
                     child_inode->clear_descendants_input_sequence_progress();
                  continue;
               }
               if (child_inode) {
                  eligible_binds.push_back(child_inode);
                  
                  bool child_conflicted = false;
                  {
                     for (auto* prior : eligible_binds) {
                        auto* prior_inode = prior->as<nodes::abstract_input_node>();
                        if (!prior_inode)
                           continue;

                        nodes::abstract_input_node* winning_node;
                        bool winner_is_not_blocked;
                        //
                        nodes::abstract_input_node::do_concurrent_nodes_conflict(
                           now,
                           *child_inode,
                           *prior_inode,
                           winning_node,
                           winner_is_not_blocked
                        );

                        nodes::abstract_input_node* losing_node = nullptr;
                        if (winning_node) {
                           losing_node = (winning_node == child_inode) ? prior_inode : child_inode;
                           //
                           if (losing_node->button_press_type != button_press_type::hold) {
                              conflict_losing_binds.push_back(losing_node);
                           }
                           if (winning_node == child_inode) {
                              conflict_losing_binds.push_back(losing_node);
                              if (!winner_is_not_blocked) {
                                 child_conflicted = true;
                              }
                           } else {
                              child_conflicted = true;
                           }
                        } else if (!winner_is_not_blocked) {
                           conflict_losing_binds.push_back(prior_inode);
                           child_conflicted = true;
                        }
                        //
                        // We don't remove the conflict loser from `eligible_binds` because we want to 
                        // test all eligible binds against both each other and any previously designated 
                        // conflict losers. We'll remove the conflict losers later, after traversal and 
                        // conflict resolution are complete.
                        // 
                        // Hm... Maybe we should rename `eligible_binds` to `binds_under_consideration` 
                        // or `accumulated_binds`...
                        //
                     }
                  }
                  //
                  if (child_conflicted) {
                     conflict_losing_binds.push_back(child_inode);
                     continue;
                  }
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
         eligible_binds.erase(
            std::remove_if(
               eligible_binds.begin(),
               eligible_binds.end(),
               [conflicted](auto* node) {
                  return node == conflicted;
               }
            ),
            eligible_binds.end()
         );
         //
         if (conflicted->button_press_type == button_press_type::hold)
            continue;
         conflicted->clear_descendants_input_sequence_progress();
      }
      conflict_losing_binds.clear();
      //
      {
         size_t hold_count = 0;
         for (const auto* node : eligible_binds)
            if (node->button_press_type == button_press_type::hold)
               ++hold_count;

         auto press_delays_hold_subalgorithm = [&conflict_losing_binds, &eligible_binds, &hold_count, now](node* current) {
            enum class result {
               stop,
               proceed,
            };

            auto recurse = [&](node* current, auto& recurse) mutable -> result {
               for (auto* child : current->child_nodes()) {
                  assert(child != nullptr);

                  if (auto* press_node = child->as<nodes::abstract_input_node>()) {
                     if (press_node->button_press_type != button_press_type::hold) {

                        for (auto* eligible : eligible_binds) {
                           if (eligible->button_press_type != button_press_type::hold)
                              continue;
                           bool result = nodes::abstract_input_node::does_press_delay_hold(
                              now,
                              *press_node,
                              *eligible
                           );
                           if (result) {
                              conflict_losing_binds.push_back(eligible);
                           }
                        }
                        if (!conflict_losing_binds.empty()) {
                           for (auto* conflicted : conflict_losing_binds) {
                              eligible_binds.erase(
                                 std::remove_if(
                                    eligible_binds.begin(),
                                    eligible_binds.end(),
                                    [conflicted, &hold_count](auto* node) {
                                       if (node == conflicted) {
                                          assert(hold_count > 0);
                                          --hold_count;
                                          return true;
                                       }
                                       return false;
                                    }
                                 ),
                                 eligible_binds.end()
                              );
                           }
                           conflict_losing_binds.clear();
                           //
                           if (hold_count == 0)
                              //
                              // We've filtered out all of the relevant binds.
                              //
                              return result::stop;
                        }

                     }
                  }

                  for (auto* item : child->child_nodes()) {
                     auto r = recurse(item, recurse);
                     if (r == result::stop)
                        return result::stop;
                  }
               }
               return result::proceed;
            };
            recurse(current, recurse);
         };
         if (hold_count) {
            press_delays_hold_subalgorithm(this->root);
         }
      }

      // Hold release.
      for (auto* hold_node : this->last_frame_active_hold_binds) {
         auto* tool_node = hold_node->as<nodes::bound_tool>();
         if (!tool_node)
            continue;
         if (std::find(eligible_binds.begin(), eligible_binds.end(), tool_node) == eligible_binds.end()) {
            tool_node->invoke_for_hold_release(hold_results);
         }
      }

      // Conflict resolution: Holds block Presses.
      eligible_binds.erase(
         std::remove_if(
            eligible_binds.begin(),
            eligible_binds.end(),
            [this, now](const auto* node) -> bool {
               if (node->button_press_type != button_press_type::hold) {
                  for (auto* hold_node : this->last_frame_active_hold_binds) {
                     bool result = nodes::abstract_input_node::does_hold_block_press(
                        *node,
                        *hold_node
                     );
                     if (result)
                        return true;
                  }
               }
               //
               // No conflict.
               //
               return false;
            }
         ),
         eligible_binds.end()
      );

      this->last_frame_active_hold_binds = {};

      // Execution;
      for (auto* node : eligible_binds) {
         if (auto* bt = node->as<nodes::bound_tool>()) {
            bt->invoke((node->button_press_type == button_press_type::hold) ? hold_results : press_results);
         }
         if (node->button_press_type == button_press_type::hold) {
            assert(node->input_sequence.state.frame_status == input_sequence::frame_status::down);
            this->last_frame_active_hold_binds.push_back(node);
         } else {
            assert(node->input_sequence.state.frame_status == input_sequence::frame_status::released);
            node->input_sequence.state.frame_status = input_sequence::frame_status::inactive;
         }
      }
   }
}