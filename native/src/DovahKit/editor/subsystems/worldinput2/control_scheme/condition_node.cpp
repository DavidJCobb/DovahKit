#include "./condition_node.h"

namespace dovahkit::subsystems::worldinput2 {
   bool control_scheme_condition_node::operator==(const control_scheme_condition_node& other) const noexcept {
      if (this->name != other.name)
         return false;
      if (this->data != other.data)
         return false;
      return true;
   }

   void control_scheme_condition_node::stream(cobb::bitstreams::reader& s) {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(this->data);
   }
   void control_scheme_condition_node::stream(cobb::bitstreams::writer& s) const {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(this->data);
   }
}