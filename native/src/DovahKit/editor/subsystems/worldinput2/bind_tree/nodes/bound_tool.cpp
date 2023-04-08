#include "bound_tool.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   void bound_tool::invoke(combined_tool_results&) const {
      // TODO
   }
   void bound_tool::invoke_for_hold_release(combined_tool_results&) const {
      // TODO
   }

   node* bound_tool::_clone_impl() const {
      auto* copy = new bound_tool;
      copy->name           = this->name;
      copy->input_sequence = this->input_sequence;
      // TODO: bound tool/function and options
      return copy;
   }
}