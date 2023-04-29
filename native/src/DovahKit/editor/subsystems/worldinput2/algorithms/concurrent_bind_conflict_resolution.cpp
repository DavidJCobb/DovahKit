#include "./concurrent_bind_conflict_resolution.h"
#include "../enums/button_press_type.h"
#include "../bind_list.h"
#include "../defaults.h"
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput2::algorithms {
   extern void concurrent_bind_conflict_resolution(
      timestamp_t current_time,
      bind_list_item& a,
      bind_list_item& b,

      bind_list_item*& winner,
      bool& allow_activation
   ) {
      winner = nullptr;
      allow_activation = true;

      const auto& seq_a = a.input_sequence;
      const auto& seq_b = b.input_sequence;
      if (seq_a == seq_b) {
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

      auto a_length = seq_a.specificity();
      auto b_length = seq_b.specificity();
      if (a_length != b_length) {
         //
         // Specificity rule: if bind U's input sequence has more buttons than bind V's input 
         // sequence, and if the two binds are in conflict, then bind U wins. The binds are 
         // in conflict if any of the input controls in V's final ISG overlap any of the 
         // terminal inputs in U.
         //
         auto& node_u = (a_length > b_length) ? a : b;
         auto& node_v = (a_length > b_length) ? b : a;
         const auto& seq_u = (a_length > b_length) ? seq_a : seq_b;
         const auto& seq_v = (a_length > b_length) ? seq_b : seq_a;

         if (seq_u.directional == seq_v.directional) {
            //
            // For directional inputs, we only care about same-frame conflicts, and here, we 
            // have one.
            //
            winner = &node_u;
            return;
         }

         auto* final_isg_v = seq_v.final_group();
         if (final_isg_v) {
            auto ti_u = seq_u.terminal_inputs();
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
               auto ti_a = seq_a.terminal_inputs();
               auto ti_b = seq_b.terminal_inputs();
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
               // The binds do not have identical terminal inputs. The binds should both be 
               // considered conflict losers; neither should be allowed to activate.
               //
               allow_activation = false;
               return;
            }
         }
      }

      // Done.
   }
}