#include "editor_mode.h"
#include "helpers/streams/bitreader.h"
#include "helpers/streams/bitwriter.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   void editor_mode::_read_impl(cobb::streams::bitreader& stream) {
      uint8_t bits = stream.read_bits(3);

      this->mode = (value_type)bits;
   }
   void editor_mode::_write_impl(cobb::streams::bitwriter& stream) const {
      stream.write_bits(3, (uint8_t)this->mode);
   }

   bool editor_mode::_compare_impl(const node& other) const {
      assert(other.type == my_type);
      const auto& casted = static_cast<const editor_mode&>(other);

      return this->mode == casted.mode;
   }
}