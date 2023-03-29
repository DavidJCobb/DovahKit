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

         void clear_descendants_progress() {
            for (auto* item : this->children) {
               switch (item->type) {
                  case node_type::bound_tool:
                  case node_type::modifier:
                     break;
                  default:
                     continue;
               }
               auto* casted = (nodes::abstract_input_node*)item;
               casted->input_sequence.clear_all_progress();
               casted->clear_descendants_progress();
            }
         }
   };
}