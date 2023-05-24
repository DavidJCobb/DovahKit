#include "editor_mode.h"
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   void editor_mode::_read_impl(cobb::bitstreams::reader& s) {
      s.stream_bits(3, mode);
   }
   void editor_mode::_write_impl(cobb::bitstreams::writer& s) const {
      s.stream_bits(3, mode);
   }

   bool editor_mode::_compare_impl(const node& other) const {
      assert(other.type == my_type);
      const auto& casted = static_cast<const editor_mode&>(other);

      return this->mode == casted.mode;
   }
}