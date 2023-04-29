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
   };
}