#include "./modifier.h"
#include <bit> // std::bit_width
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   node* modifier::_clone_impl() const {
      auto* copy = new modifier;
      copy->name           = this->name;
      copy->input_sequence = this->input_sequence;
      return copy;
   }
   void modifier::_read_impl(cobb::bitstreams::reader& s) {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(input_sequence);
   }
   void modifier::_write_impl(cobb::bitstreams::writer& s) const {
      s.stream<std::bit_width(max_name_length)>(name);
      s.stream(input_sequence);
   }

   bool modifier::_compare_impl(const node& other) const {
      assert(other.type == my_type);
      const auto& casted = static_cast<const modifier&>(other);

      if (this->name != casted.name)
         return false;
      if (this->input_sequence != casted.input_sequence)
         return false;

      return true;
   }
}