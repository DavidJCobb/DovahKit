#include "tree.h"
#include <algorithm>
#include <QDebug>
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

      auto recursively_disqualify_node_and_descendants = [&eligible_binds](node& target, bool clear_progress = false) -> size_t {
         size_t removed = 0;
         auto   recurse = [&](node& target, auto& recurse) mutable -> void {
            if (auto* casted = target.as<nodes::abstract_input_node>()) {
               eligible_binds.erase(
                  std::remove_if(
                     eligible_binds.begin(),
                     eligible_binds.end(),
                     [casted, &removed](auto* node) {
                        ++removed;
                        return node == casted;
                     }
                  ),
                  eligible_binds.end()
               );
               if (casted->button_press_type != button_press_type::hold) {
                  casted->clear_descendants_input_sequence_progress();
               }
               if (eligible_binds.empty())
                  return;
            }
            for (auto* child : target.child_nodes()) {
               recurse(*child, recurse);
               if (eligible_binds.empty())
                  return;
            }
         };
         recurse(target, recurse);
         return removed;
      };

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

                        if (!matched && child_inode->button_press_type == button_press_type::hold) {
                           child_inode->state.press_blocked_hold = false;
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
               if (child->is_valid_parent())
                  recurse(child, recurse);
            }
            //
         };
         recurse(current, recurse);
      };
      //
      traversal_subalgorithm(this->root);
      
      for (auto* conflicted : conflict_losing_binds) {
         recursively_disqualify_node_and_descendants(*conflicted, true);
      }
      conflict_losing_binds.clear();

      // Press-preempts-Hold.
      [this, now, &device, &eligible_binds, &recursively_disqualify_node_and_descendants](){
         struct hold_node_conflict_info {
            nodes::abstract_input_node* node = nullptr;
            size_t outlasted  = 0;
            size_t delayed_by = 0;
            bool   is_advanced_past : 1 = false;
            bool   is_blocked       : 1 = false;
         };
         std::vector<hold_node_conflict_info> seen_hold_nodes;
         std::vector<nodes::abstract_input_node*> winning_press_nodes;

         for (auto* node : eligible_binds) {
            if (node->button_press_type == button_press_type::hold) {
               auto& item = seen_hold_nodes.emplace_back();
               item.node = node;
               //
               if (node->state.press_blocked_hold)
                  item.is_blocked = true;
            }
         }
         if (seen_hold_nodes.empty())
            return;

         enum class pass_result {
            stop,
            proceed,
         };
         auto pass_1 = [&](node* current, auto& recurse) mutable -> void {
            for (auto* child : current->child_nodes()) {
               assert(child != nullptr);

               if (auto* press_node = child->as<nodes::abstract_input_node>()) {
                  if (press_node->button_press_type != button_press_type::hold) {

                     for (auto& hold_info : seen_hold_nodes) {
                        auto result = nodes::abstract_input_node::does_press_delay_hold(
                           now,
                           device,
                           *press_node,
                           *hold_info.node
                        );
                        switch (result) {
                           using enum nodes::abstract_input_node::press_preempt_hold_result;
                           case press_delays_hold:
                              ++hold_info.delayed_by;
                              winning_press_nodes.push_back(press_node);
                              break;
                           case press_blocks_hold:
                              hold_info.is_blocked = true;
                              break;
                           case press_advanced_past_hold:
                              ++hold_info.delayed_by;
                              hold_info.is_advanced_past = true;
                              winning_press_nodes.push_back(press_node);
                              break;
                           case hold_outlasted_press:
                              ++hold_info.outlasted;
                              break;
                        }
                     }
                  }
               }

               for (auto* item : child->child_nodes()) {
                  recurse(item, recurse);
               }
            }
         };
         pass_1(this->root, pass_1);

         {
            std::vector<nodes::abstract_input_node*> retroamended_losing_presses;
            for (auto& hold_info : seen_hold_nodes) {
               if (hold_info.is_blocked || hold_info.is_advanced_past)
                  continue;
               if (hold_info.outlasted == 0)
                  continue;
               hold_info.outlasted += hold_info.delayed_by;
               hold_info.delayed_by = 0;
               //
               for (auto* press_node : winning_press_nodes) {
                  auto result = nodes::abstract_input_node::does_press_delay_hold(
                     now,
                     device,
                     *press_node,
                     *hold_info.node
                  );
                  switch (result) {
                     using enum nodes::abstract_input_node::press_preempt_hold_result;
                     case press_delays_hold:
                     case hold_outlasted_press:
                        retroamended_losing_presses.push_back(press_node);
                        break;
                  }
               }
            }
            if (!retroamended_losing_presses.empty()) {
               winning_press_nodes.erase(
                  std::remove_if(
                     winning_press_nodes.begin(),
                     winning_press_nodes.end(),
                     [&retroamended_losing_presses](const auto* node) -> bool {
                        return std::find(retroamended_losing_presses.begin(), retroamended_losing_presses.end(), node) != retroamended_losing_presses.end();
                     }
                  ),
                  winning_press_nodes.end()
               );
            }
         }
         
         for (auto& hold_info : seen_hold_nodes) {
            if (hold_info.is_blocked)
               continue;
            if (hold_info.delayed_by == 0)
               continue;
            //
            bool any_loss = false;
            bool still_advanced_past = false;
            for (auto* press_node : winning_press_nodes) {
               auto result = nodes::abstract_input_node::does_press_delay_hold(
                  now,
                  device,
                  *press_node,
                  *hold_info.node
               );
               switch (result) {
                  using enum nodes::abstract_input_node::press_preempt_hold_result;
                  case press_delays_hold:
                     any_loss = true;
                     break;
                  case press_advanced_past_hold:
                     any_loss = true;
                     still_advanced_past = true;
                     break;
               }
            }
            if (!any_loss) {
               hold_info.delayed_by = 0;
               if (!still_advanced_past) {
                  hold_info.is_advanced_past = false;
               }
            }
         }

         for (auto& hold_info : seen_hold_nodes) {
            auto* node = hold_info.node;
            if (hold_info.delayed_by == 0 && !hold_info.is_blocked && !hold_info.is_advanced_past) {
               if (!node->state.press_blocked_hold)
                  continue;
            }
            recursively_disqualify_node_and_descendants(*hold_info.node, false);
         }
      }();

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
      // NOTE: Modifiers are always Hold binds, whereas we're here removing Press and Long Press 
      // binds, so we don't here need to worry about recursively removing descendants from the 
      // list of eligible binds if a Press or Long Press bind loses a conflict.

      this->last_frame_active_hold_binds = {};

      // Execution:
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