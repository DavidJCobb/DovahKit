#include "./modifier.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   node* modifier::_clone_impl() const {
      auto* copy = new modifier;
      copy->name              = this->name;
      copy->button_press_type = this->button_press_type;
      copy->input_sequence    = this->input_sequence;
      return copy;
   }
}