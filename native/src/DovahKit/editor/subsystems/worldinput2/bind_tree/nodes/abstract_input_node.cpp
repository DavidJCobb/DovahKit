#include "./abstract_input_node.h"

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
}