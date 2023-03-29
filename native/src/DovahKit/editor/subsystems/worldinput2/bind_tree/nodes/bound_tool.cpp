#include "bound_tool.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   node* bound_tool::_clone_impl() const {
      auto* copy = new bound_tool;
      copy->name           = this->name;
      copy->input_sequence = this->input_sequence;
      // TODO: bound tool/function and options
      return copy;
   }
}