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
   node* modifier::_read_impl(cobb::streams::bitreader& stream) {
      stream.read(name, input_sequence);
   }
   node* modifier::_write_impl(cobb::streams::bitwriter& stream) const {
      stream.write(name, input_sequence);
   }
}