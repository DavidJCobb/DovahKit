#pragma once
#include "../node.h"
#include "../../enums/button_press_type.h"
#include "../../input_sequence.h"

namespace dovahkit::subsystems::worldinput2::devices {
   class abstract_device_handler;
}

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   class abstract_input_node : public node {
      protected:
         abstract_input_node(node_type t) : node(t) {}

      public:
         QString name;
         typename button_press_type button_press_type = button_press_type::press;
         typename input_sequence    input_sequence;
         mutable struct {
            bool press_blocked_hold : 1 = false; // cross-frame Hold node state for Press-preempts-Hold
         } state;

         class input_sequence absolute_input_sequence() const;
         std::vector<inputs::button> absolute_terminal_inputs() const {
            return this->absolute_input_sequence().terminal_inputs();
         }

         enum class press_preempt_hold_result {
            no_conflict,
            press_delays_hold,
            press_blocks_hold,
            press_advanced_past_hold,
            hold_outlasted_press,
         };
         static press_preempt_hold_result does_press_delay_hold(
            timestamp_t current_time,
            devices::abstract_device_handler&,
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