#include "./modifier.h"
#include <bit>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace dovahkit::subsystems::worldinput {
   bool control_scheme_modifier::operator==(const control_scheme_modifier& other) const noexcept {
      if (this->name != other.name)
         return false;
      if (this->input_sequence != other.input_sequence)
         return false;
      return true;
   }

   void control_scheme_modifier::stream(cobb::bitstreams::reader& s) {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(input_sequence);
   }
   void control_scheme_modifier::stream(cobb::bitstreams::writer& s) const {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(input_sequence);
   }

   void control_scheme_modifier::assert_validity() const {
      assert(this->name.size() <= max_name_length);
      this->input_sequence.assert_validity();
   }
}