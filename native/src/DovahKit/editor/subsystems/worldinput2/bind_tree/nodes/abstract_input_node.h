#pragma once
#include "../node.h"
#include "../../enums/button_press_type.h"
#include "../../input_sequence.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class abstract_input_node : public node {
      protected:
         abstract_input_node(node_type t) : node(t) {}

      public:
         QString name;
         typename button_press_type button_press_type = button_press_type::press;
         typename input_sequence    input_sequence;

         class input_sequence absolute_input_sequence() const;

         static bool does_press_delay_hold(
            timestamp_t current_time,
            const abstract_input_node& press,
            const abstract_input_node& hold
         );
         static void do_concurrent_nodes_conflict(
            timestamp_t current_time,
            abstract_input_node& a,
            abstract_input_node& b,

            abstract_input_node*& winner,
            bool& allow_activation
         );
         static bool does_hold_block_press(
            const abstract_input_node& press,
            const abstract_input_node& hold
         );
   };
}