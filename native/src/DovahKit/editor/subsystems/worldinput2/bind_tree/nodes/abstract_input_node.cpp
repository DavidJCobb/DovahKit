#include "./abstract_input_node.h"
#include "helpers/unreachable.h"
#include "../../devices/abstract_device_handler.h"
#include "../../defaults.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class input_sequence abstract_input_node::absolute_input_sequence() const {
      for (auto* ancestor = this->parent_node(); ancestor; ancestor = ancestor->parent_node()) {
         if (auto* casted = ancestor->as<abstract_input_node>()) {
            //
            // Recursion and using operator<<= means that we'll clone just the outermost 
            // input sequence, and then modify it with the descendant sequences. Instead, 
            // we could do something like this:
            // 
            //  - At the start of this function, clone our input sequence and store it in 
            //    a variable, `absolute`.
            // 
            //  - Traverse up our ancestors without recursion.
            // 
            //  - When we encounter an input-node ancestor, overwrite absolute with the 
            //    expression `casted->input_sequence.clone() << absolute`.
            // 
            // But that would result in multiple clones, which -- until we implement flat 
            // ISG storage -- will mean tons of redundant heap allocations and frees.
            //
            return (casted->absolute_input_sequence() <<= this->input_sequence);
         }
      }
      return this->input_sequence.clone();
   }

   /*static*/ bool abstract_input_node::does_press_delay_hold(
      timestamp_t current_time,
      devices::abstract_device_handler& device,
      const abstract_input_node& press,
      const abstract_input_node& hold
   ) {
      if (hold.state.outlasted_press_delays_hold) {
         press.state.press_delay_was_outlasted = true;
         return false;
      }
      if (hold.state.press_blocked_hold) {
         return true;
      }

      using frame_status = input_sequence::frame_status;
      using group        = input_sequence::group;
      using group_type   = input_sequence::group_type;

      const auto abs_p = press.absolute_input_sequence();
      const auto abs_h = hold.absolute_input_sequence();

      const auto* final_h = abs_h.final_group();
      if (!final_h) {
         return false;
      }
      assert(final_h->type == group_type::single_control || final_h->type == group_type::concurrent_unordered);

      std::vector<inputs::button> keys_h;
      final_h->terminal_inputs(keys_h);

      if (press.input_sequence.state.frame_status == input_sequence::frame_status::released) {
         //
         // The Press node was released. If it conflicts with the Hold node, then assume that 
         // the Hold node was delayed and the Press node was released; that release should now 
         // block the Hold node (i.e. Press-blocks-Hold).
         //
         auto keys_p = abs_p.terminal_inputs();
         bool subset = true;
         for (const auto& a : keys_h) {
            bool found = false;
            for (const auto& b : keys_p) {
               if (a == b) {
                  found = true;
                  break;
               }
            }
            if (!found) {
               subset = false;
               break;
            }
         }
         if (subset) {
            hold.state.press_blocked_hold = true;
            return true;
         }
      }

      struct key_in_press_node {
         inputs::button button;

         bool passed = false;
         // If all conflicting keys are `passed`, then this means that the user has advanced 
         // past all of them. The Hold node should be delayed indefinitely, but should not be 
         // blocked outright. This is because if the user releases keys exclusive to the Press 
         // node's input sequence, they may "rewind" before a conflicting key, thereby allowing 
         // the Hold node to potentially win this conflict again.
         // 
         // If any conflicting keys exist and are not `passed`, then a conflict is present, and 
         // the Hold node should experience a finite delay before activating (with its activation 
         // then preempting activation of the conflicting Press node).
      };
      //
      std::vector<key_in_press_node> keys_p;

      auto key_gathering_subalgorithm = [&keys_p, current_time, &device](const group& current) {
         struct result {
            frame_status status    = frame_status::inactive;
            timestamp_t  timestamp = zero_timestamp;
         };

         auto recurse = [&](const group& current, auto& recurse) -> result {
            if (current.type == group_type::single_control) {
               auto bs = device.get_state_of(current.button);
               if (bs.is_down()) {
                  keys_p.push_back({ .button = current.button, .passed = false });
                  return { frame_status::down, bs.down_when };
               }
               return { frame_status::inactive, zero_timestamp };
            }
            if (current.type == group_type::concurrent_ordered) {
               auto   previous_timestamp = zero_timestamp;
               size_t previous_start     = keys_p.size();
               for (const auto* child : current.children) {
                  size_t prior_count = keys_p.size();
                  auto   result      = recurse(*child, recurse);
                  if (result.status == frame_status::down) {
                     if (result.timestamp < previous_timestamp) {
                        keys_p.resize(prior_count);
                        return { frame_status::inactive, previous_timestamp };
                     }
                     //
                     // Mark the previous-sibling ISG's contributions to `keys_p` (if any) as passed.
                     //
                     for (size_t j = previous_start; j < prior_count; ++j) {
                        keys_p[j].passed = true;
                     }
                  } else {
                     return { frame_status::inactive, previous_timestamp };
                  }
                  previous_timestamp = result.timestamp;
                  previous_start     = prior_count;
               }
               return { frame_status::down, previous_timestamp };
            }
            if (current.type == group_type::concurrent_unordered) {
               size_t additions_start_at = keys_p.size();
               
               bool any_not_down = false;
               auto most_recent  = zero_timestamp;
               for (const auto* child : current.children) {
                  size_t prior_count = keys_p.size();
                  auto   result      = recurse(*child, recurse);
                  if (result.status != frame_status::down)
                     any_not_down = true;
                  if (result.timestamp > most_recent) {
                     most_recent = result.timestamp;
                     //
                     // The keys we just found were pressed more recently than the previous-sibling 
                     // ISGs. Mark those previous-sibling ISGs' contributions to `keys_p` as passed.
                     //
                     for (size_t j = additions_start_at; j < prior_count; ++j) {
                        keys_p[j].passed = true;
                     }
                     additions_start_at = prior_count; // hack, to avoid redundant work if we end up running the above loop multiple times
                  } else {
                     //
                     // The keys we just found were pressed less recently than a previous-sibling 
                     // ISG. Mark them all as passed.
                     //
                     for (size_t j = prior_count; j < keys_p.size(); ++j) {
                        keys_p[j].passed = true;
                     }
                  }
               }
               if (any_not_down)
                  return { frame_status::inactive, most_recent };
               return { frame_status::down, most_recent };
            }
            if (current.type == group_type::separated_ordered) {
               if (current.children.empty()) {
                  return { frame_status::inactive, zero_timestamp };
               }
               //
               const auto& child = current.current_item();
               auto result = recurse(child, recurse);
               if (result.status == frame_status::down && &child == current.children.back()) {
                  return { frame_status::down, current_time };
               }
               return { frame_status::inactive, result.timestamp };
            }
            cobb::unreachable();
         };
         recurse(current, recurse);
      };
      key_gathering_subalgorithm(*(press.input_sequence.root));

      bool all_passed   = true;
      bool any_conflict = false;
      for (const auto& key_p : keys_p) {
         for (const auto& key_h : keys_h) {
            if (key_p.button == key_h) {
               any_conflict = true;
               if (!key_p.passed) {
                  all_passed = false;
               }
               break;
            }
         }
      }
      if (!any_conflict) {
         return false;
      }
      press.state.press_did_delay_hold = true;
      if (all_passed) {
         // indefinite delay
         return true;
      }

      auto elapsed_h = elapsed_time(hold.input_sequence.state.went_down_at, current_time);
      {
         auto disambig = defaults::press_to_hold_threshold;
         if (press.button_press_type == button_press_type::long_press)
            disambig += defaults::press_to_long_press_threshold;

         if (elapsed_h > disambig) {
            //
            // The Hold bind has been pressed down for long enough to win a conflict.
            //
            hold.state.outlasted_press_delays_hold = true;
            press.state.press_delay_was_outlasted = true;
            return false;
         }
      }
      return true;
   }
   /*static*/ void abstract_input_node::do_concurrent_nodes_conflict(
      timestamp_t current_time,
      abstract_input_node& a,
      abstract_input_node& b,

      abstract_input_node*& winner,
      bool& allow_activation
   ) {
      winner = nullptr;
      allow_activation = true;

      auto abs_a = a.absolute_input_sequence();
      auto abs_b = b.absolute_input_sequence();
      if (abs_a == abs_b) {
         if (a.button_press_type == b.button_press_type) {
            // Identical binds rule:  If the absolute input sequences and press types are 
            // identical, then the binds do not conflict; both nodes should be allowed to 
            // activate in tandem.
            return;
         }
         if (a.button_press_type != button_press_type::hold && b.button_press_type != button_press_type::hold) {
            // Press duration rule: If the absolute input sequences are identical, but one 
            // bind is a Press and the other is a Long Press, then the Long Press wins the 
            // conflict if it was down long enough; else, the Press.
            auto& press  = a.button_press_type == button_press_type::press ? a : b;
            auto& longer = (&press == &a) ? b : a;
            //
            if (elapsed_time(longer.input_sequence.state.went_down_at, current_time) >= defaults::press_to_long_press_threshold) {
               winner = &longer;
               return;
            }
            winner = &press;
            return;
         }
      }

      if ((a.button_press_type == button_press_type::hold) != (b.button_press_type == button_press_type::hold)) {
         return;
      }

      auto a_length = abs_a.specificity();
      auto b_length = abs_b.specificity();
      if (a_length != b_length) {
         //
         // Specificity rule: if node U's input sequence has more buttons than node V's input 
         // sequence, and if the two nodes are in conflict, then node U wins. The nodes are 
         // in conflict if any of the input controls in V's final ISG overlap any of the 
         // terminal inputs in U.
         //
         auto& node_u = (a_length > b_length) ? a : b;
         auto& node_v = (a_length > b_length) ? b : a;
         const auto& abs_u = (a_length > b_length) ? abs_a : abs_b;
         const auto& abs_v = (a_length > b_length) ? abs_b : abs_a;

         auto* final_isg_v = abs_v.final_group();
         if (final_isg_v) {
            auto ti_u = abs_u.terminal_inputs();
            if (final_isg_v->type == input_sequence::group_type::single_control) {
               for (const auto& term : ti_u) {
                  if (term == final_isg_v->button) {
                     //
                     // The final ISG in V is a single input control, and is among the input 
                     // controls present in U's terminal inputs. A conflict is present, and 
                     // U, being the longer input sequence, wins.
                     //
                     winner = &node_u;
                     return;
                  }
               }
            } else {
               for (const auto& term : ti_u) {
                  if (final_isg_v->is_or_contains_input_control(term)) {
                     //
                     // The final ISG in V is a group, and one of the input controls somewhere 
                     // inside of that group is present in U's terminal inputs. A conflict is 
                     // present, and U, being the longer input sequence, wins.
                     //
                     winner = &node_u;
                     return;
                  }
               }
            }
         }
      } else {
         // Modifier conflict rule.
         if (a.button_press_type != button_press_type::hold && b.button_press_type != button_press_type::hold) {
            bool overlap = false;
            {
               auto ti_a = abs_a.terminal_inputs();
               auto ti_b = abs_b.terminal_inputs();
               for (const auto& item_a : ti_a) {
                  for (const auto& item_b : ti_b) {
                     if (item_a == item_b) {
                        overlap = true;
                        break;
                     }
                  }
                  if (overlap)
                     break;
               }
            }
            if (overlap) {
               //
               // The nodes do not have identical terminal inputs. The nodes should both be 
               // considered conflict losers; neither should be allowed to activate.
               //
               allow_activation = false;
               return;
            }
         }
      }

      // Done.
   }
   /*static*/ bool abstract_input_node::does_hold_block_press(
      const abstract_input_node& press,
      const abstract_input_node& hold
   ) {
      // A conflict is present if the absolute terminal inputs of the two binds overlap.
      auto abs_p = press.absolute_input_sequence().terminal_inputs();
      auto abs_h = hold.absolute_input_sequence().terminal_inputs();
      //
      for (const auto& item_p : abs_p)
         for (const auto& item_h : abs_h)
            if (item_p == item_h)
               return true;
      
      return false;
   }
}