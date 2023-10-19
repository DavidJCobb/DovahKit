#include "./press_preempts_hold.h"
#include "helpers/unreachable.h"
#include "../bind_list.h"
#include "../config.h"
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput::algorithms {
   extern press_preempt_hold_result press_preempts_hold(
      timestamp_t current_time,
      devices::abstract_device_handler& device,
      const bind_list_item& press,
      const bind_list_item& hold
   ) {
      using frame_status = input_sequence::frame_status;
      using group        = input_sequence::group;
      using group_type   = input_sequence::group_type;

      const auto& seq_p = press.input_sequence;
      const auto& seq_h = hold.input_sequence;

      const auto* final_h = seq_h.final_group();
      if (!final_h) {
         return press_preempt_hold_result::no_conflict;
      }
      assert(final_h->type == group_type::single_control || final_h->type == group_type::concurrent_unordered);

      std::vector<inputs::button> keys_h;
      final_h->terminal_inputs(keys_h);

      if (press.input_sequence.state.frame_status == input_sequence::frame_status::released) {
         //
         // The Press bind was released. If it conflicts with the Hold bind, then assume that 
         // the Hold bind was delayed and the Press bind was released; that release should now 
         // block the Hold bind (i.e. Press-blocks-Hold).
         //
         auto keys_p = seq_p.terminal_inputs();
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
            return press_preempt_hold_result::press_blocks_hold;
         }
      }

      //
      // Below: Press-delays-Hold or no conflict.
      //

      struct key_in_press_node {
         inputs::button button;

         bool passed = false;
         // If all conflicting keys are `passed`, then this means that the user has advanced 
         // past all of them. The Hold bind should be delayed indefinitely, but should not be 
         // blocked outright. This is because if the user releases keys exclusive to the Press 
         // bind's input sequence, they may "rewind" before a conflicting key, thereby allowing 
         // the Hold bind to potentially win this conflict again.
         // 
         // If any conflicting keys exist and are not `passed`, then a conflict is present, and 
         // the Hold bind should experience a finite delay before activating (with its activation 
         // then preempting activation of the conflicting Press bind).
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
         return press_preempt_hold_result::no_conflict;
      }
      if (hold.input_sequence.has_range_requirement()) {
         //
         // The Hold bind has a range constraint, and the constraint was met (it must have been, 
         // for us to end up seeing that bind here). Allow it to win this conflict by default.
         //
         return press_preempt_hold_result::hold_won_via_range;
      }
      if (all_passed) {
         // indefinite delay
         return press_preempt_hold_result::press_advanced_past_hold;
      }

      auto elapsed_h = elapsed_time(hold.input_sequence.state.went_down_at, current_time);
      {
         auto disambig = config::press_to_hold_threshold();
         if (press.button_press_type == button_press_type::long_press)
            disambig += config::press_to_long_press_threshold();

         if (elapsed_h > disambig) {
            //
            // The Hold bind has been pressed down for long enough to win a conflict.
            //
            return press_preempt_hold_result::hold_outlasted_press;
         }
      }
      return press_preempt_hold_result::press_delays_hold;
   }
}