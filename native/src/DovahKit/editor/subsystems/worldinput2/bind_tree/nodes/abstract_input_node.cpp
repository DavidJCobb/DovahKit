#include "./abstract_input_node.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   /*static*/ void abstract_input_node::do_conflict_resolution(
      abstract_input_node& a,
      abstract_input_node& b,

      abstract_input_node*& winner,
      bool& allow_activation
   ) {
      winner = nullptr;
      allow_activation = true;

      const auto absolute_a = a.absolute_input_sequence();
      const auto absolute_b = b.absolute_input_sequence();

      if (a.button_press_type == b.button_press_type) {
         // Superset/subset rule.
         if (absolute_a.is_subset_of(absolute_b)) {
            winner = &a;
            return;
         }

         // Identical binds case.
         if (absolute_a == absolute_b) {
            winner = nullptr;
            return;
         }
      }

      // Same press type case.
      if (static_assert(false, "TODO: re-spec")) {
         //
         // The intention is to handle a conflict between Press [LS + X] and Press [B + X] 
         // when the user presses and holds both B and LS, and then presses X. Behavior in 
         // this situation is that neither bind should activate.
         // 
         // The specified check for this case is wrong, however. The spec says to compare 
         // both sequences to see if their absolute terminal inputs are equal, but... they 
         // wouldn't be! We'd end up comparing [LS, X] to [B, X]. I think what we want is 
         // to compare their *last* terminal input for equality.
         // 
         // The spec is wrong here. If we check whether the two binds have the same 
         // absolute terminal inputs, then we fail to handle the case above. I think instead, 
         // we'd want to check if the absolute terminal inputs overlap but are not identical.
         //
         winner = nullptr;
         allow_activation = false;
         return;
      }

      if (a.button_press_type != b.button_press_type) {
         double a_time = static_assert(false, "TODO: compute how long A was down before it got released");
         double b_time = static_assert(false, "TODO: compute how long B was down before it got released");
         if (a.button_press_type == button_press_type::hold || b.button_press_type == button_press_type::hold) {

            // Hold/non-Hold case.

            static_assert(false, "TODO");
            return;
         }

         auto& long_press_node = (a.button_press_type == button_press_type::long_press) ? a : b;
         auto& press_node      = (a.button_press_type != button_press_type::long_press) ? a : b;

      }

      // No conflict.
      return;
   }
}