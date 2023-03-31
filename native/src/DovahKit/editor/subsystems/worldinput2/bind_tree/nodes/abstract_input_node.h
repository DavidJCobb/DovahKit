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

         class input_sequence absolute_input_sequence() const {
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

         static void do_conflict_resolution(
            abstract_input_node& a,
            abstract_input_node& b,

            abstract_input_node*& winner,
            bool& allow_activation
         );
   };
}