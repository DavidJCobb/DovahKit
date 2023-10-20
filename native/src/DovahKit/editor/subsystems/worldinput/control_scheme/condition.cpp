#include "./condition.h"

namespace dovahkit::subsystems::worldinput {
   bool control_scheme_condition::operator==(const control_scheme_condition& other) const noexcept {
      if (this->name != other.name)
         return false;
      if (this->data != other.data)
         return false;
      return true;
   }

   void control_scheme_condition::stream(cobb::bitstreams::reader& s) {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(this->data);
   }
   void control_scheme_condition::stream(cobb::bitstreams::writer& s) const {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(this->data);
   }
}