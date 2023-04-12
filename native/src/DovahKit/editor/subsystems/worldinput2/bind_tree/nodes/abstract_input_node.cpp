#include "./abstract_input_node.h"
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
      const abstract_input_node& press,
      const abstract_input_node& hold
   ) {
      // A conflict is present if both binds have overlapping absolute terminal inputs.
      auto abs_p = press.absolute_input_sequence().terminal_inputs();
      auto abs_h = hold.absolute_input_sequence().terminal_inputs();
      //
      {
         bool overlap = false;
         for (const auto& item_p : abs_p) {
            for (const auto& item_h : abs_h) {
               if (item_p == item_h) {
                  overlap = true;
                  break;
               }
            }
            if (overlap)
               break;
         }
         //
         if (!overlap)
            return false;
      }

      // The two binds may conflict. Next, we need to check the timestamps at which 
      // they went down.

      auto elapsed_p = elapsed_time(press.input_sequence.state.went_down_at, current_time);
      auto elapsed_h = elapsed_time(hold.input_sequence.state.went_down_at,  current_time);

      auto disambig = defaults::press_to_hold_threshold;
      if (press.button_press_type == button_press_type::long_press)
         disambig += defaults::press_to_long_press_threshold;

      if (elapsed_h > disambig) {
         return false;
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