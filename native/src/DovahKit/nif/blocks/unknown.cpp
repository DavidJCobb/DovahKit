#include "unknown.h"
#include "../reader.h"

namespace nifDK::block_types {
   void unknown_block::parse(file_reader& reader) {
      this->data.resize(reader.size());
      reader.unchecked_read(this->data.data(), this->data.size());
   }
}