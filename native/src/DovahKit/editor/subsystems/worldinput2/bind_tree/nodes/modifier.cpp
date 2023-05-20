#include "./modifier.h"
#include "helpers/streams/bitreader.h"
#include "helpers/streams/bitwriter.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   node* modifier::_clone_impl() const {
      auto* copy = new modifier;
      copy->name           = this->name;
      copy->input_sequence = this->input_sequence;
      return copy;
   }
   void modifier::_read_impl(cobb::streams::bitreader& stream) {
      stream.read(name, input_sequence);
   }
   void modifier::_write_impl(cobb::streams::bitwriter& stream) const {
      stream.write(name, input_sequence);
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