#include "bound_tool.h"

#include "../../debugging.h"
#include "../../tools/combined_tool_results.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   void bound_tool::invoke(combined_tool_results& out) const {
      // TODO
      //
      if constexpr (enable_initial_testing) {
         out.data += this->name.toStdString().c_str();
         out.data += '\n';
      }
   }
   void bound_tool::invoke_for_hold_release(combined_tool_results& out) const {
      // TODO
      //
      if constexpr (enable_initial_testing) {
         out.data += "[Hold-Release] ";
         out.data += this->name.toStdString().c_str();
         out.data += '\n';
      }
   }

   node* bound_tool::_clone_impl() const {
      auto* copy = new bound_tool;
      copy->name              = this->name;
      copy->button_press_type = this->button_press_type;
      copy->input_sequence    = this->input_sequence;
      // TODO: bound tool/function and options
      return copy;
   }
}