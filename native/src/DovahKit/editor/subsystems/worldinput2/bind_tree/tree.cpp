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
                           child_inode->state.outlasted_press_delays_hold = false;
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
         recursively_disqualify_node_and_descendants(*conflicted, true);
      }
      conflict_losing_binds.clear();

      // Press-preempts-Hold.
      [this, now, &device, &eligible_binds, &recursively_disqualify_node_and_descendants](){
         size_t hold_count = 0;
         for (const auto* node : eligible_binds) {
            if (node->button_press_type == button_press_type::hold) {
               node->state.press_delayed_hold = false;
               ++hold_count;
            }
         }
         if (hold_count == 0) {
            return;
         }

         struct losing_hold_node_entry {
            nodes::abstract_input_node* node = nullptr;
            size_t lost_to_count = 0;

            constexpr losing_hold_node_entry() {}
            constexpr losing_hold_node_entry(nodes::abstract_input_node* n, size_t c) : node(n), lost_to_count(c) {}
         };
         std::vector<losing_hold_node_entry> blocked_or_delayed_hold_nodes;

         enum class pass_result {
            stop,
            proceed,
         };
         auto pass_1 = [&](node* current, auto& recurse) mutable -> pass_result {
            for (auto* child : current->child_nodes()) {
               assert(child != nullptr);

               if (auto* press_node = child->as<nodes::abstract_input_node>()) {
                  if (press_node->button_press_type != button_press_type::hold) {

                     press_node->state.press_did_delay_hold      = false;
                     press_node->state.press_delay_was_outlasted = false;

                     for (auto* eligible : eligible_binds) {
                        if (eligible->button_press_type != button_press_type::hold)
                           continue;
                        //
                        if (eligible->state.outlasted_press_delays_hold) {
                           press_node->state.press_delay_was_outlasted = true;
                           continue;
                        }
                        //
                        bool result = nodes::abstract_input_node::does_press_delay_hold(
                           now,
                           device,
                           *press_node,
                           *eligible
                        );
                        if (result) {
//qDebug("Press-preempts-Hold: Hold node lost: %s", qUtf8Printable(eligible->name));
                           bool found = false;
                           for (auto& item : blocked_or_delayed_hold_nodes) {
                              if (item.node == eligible) {
                                 found = true;
                                 ++item.lost_to_count;
                                 break;
                              }
                           }
                           if (!found)
                              blocked_or_delayed_hold_nodes.emplace_back(eligible, 1);
                        }
                     }
                     if (blocked_or_delayed_hold_nodes.size() == hold_count) {
                        //
                        // We've filtered out all of the relevant binds.
                        //
                        return pass_result::stop;
                     }
                  }
               }

               for (auto* item : child->child_nodes()) {
                  auto r = recurse(item, recurse);
                  if (r == pass_result::stop)
                     return pass_result::stop;
               }
            }
            return pass_result::proceed;
         };
         pass_1(this->root, pass_1);

         if (blocked_or_delayed_hold_nodes.empty())
            return;

         std::vector<nodes::abstract_input_node*> un_losers;
         //
         auto pass_2 = [&](node* current, auto& recurse) -> pass_result {
            for (auto* child : current->child_nodes()) {
               assert(child != nullptr);

               if (auto* press_node = child->as<nodes::abstract_input_node>()) {
                  if (press_node->button_press_type != button_press_type::hold) {

                     if (press_node->state.press_did_delay_hold && press_node->state.press_delay_was_outlasted) {
                        //
                        // This Press node won one Press-delays-Hold conflict, but lost another. Because it lost 
                        // the other, the Hold node it lost against should activate... which should preempt the 
                        // Press node, and so the other Hold nodes that lost against it should "un-lose."
                        //
                        for (auto& loser : blocked_or_delayed_hold_nodes) {
                           if (loser.lost_to_count == 0)
                              continue;
                           bool result = nodes::abstract_input_node::does_press_delay_hold(
                              now,
                              device,
                              *press_node,
                              *loser.node
                           );
                           if (result) {
//qDebug("Press-preempts-Hold: Hold node un-lost against a Press node: %s", qUtf8Printable(loser.node->name));
                              --loser.lost_to_count;
                              if (loser.lost_to_count == 0) {
//qDebug("Press-preempts-Hold: Hold node un-lost entirely and will remain eligible: %s", qUtf8Printable(loser.node->name));
                                 un_losers.push_back(loser.node);

                                 if (un_losers.size() == blocked_or_delayed_hold_nodes.size()) {
                                    return pass_result::stop;
                                 }
                              }
                           }
                        }
                     }

                  }
               }

               for (auto* item : child->child_nodes()) {
                  auto code = recurse(item, recurse);
                  if (code == pass_result::stop)
                     return pass_result::stop;
               }
            }
            return pass_result::proceed;
         };
         pass_2(this->root, pass_2);

         if (!un_losers.empty()) {
            blocked_or_delayed_hold_nodes.erase(
               std::remove_if(
                  blocked_or_delayed_hold_nodes.begin(),
                  blocked_or_delayed_hold_nodes.end(),
                  [&un_losers](const auto& item) -> bool {
                     return std::find(un_losers.begin(), un_losers.end(), item.node) != un_losers.end();
                  }
               ),
               blocked_or_delayed_hold_nodes.end()
            );
         }

         for (const auto& conflicted : blocked_or_delayed_hold_nodes) {
//qDebug("Press-preempts-Hold: disqualified: %s", qUtf8Printable(conflicted.node->name));
            recursively_disqualify_node_and_descendants(*conflicted.node, false);
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