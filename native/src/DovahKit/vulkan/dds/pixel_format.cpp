#include "pixel_format.h"
#include "helpers/generic_reader_ex.h"
#include "load_exception.h"

namespace vulkanDK::dds {
   void pixel_format::read(cobb::generic_reader_ex& reader) {
      reader.require_size(serialized_size);
      reader.unchecked_read(this->size);
      if (this->size != pixel_format::serialized_size)
         throw load_exception("Invalid pixel format (serialized size is wrong).");
      reader.unchecked_read(this->flags);
      reader.unchecked_read<std::endian::big>(this->four_cc);
      reader.unchecked_read(this->rgb_bitcount);
      reader.unchecked_read(this->channel_masks.list);
   }
}