#include "NiStringExtraData.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiStringExtraData::parse(file_reader& reader) {
      NiExtraData::parse(reader);
      if (reader.version() <= file_version::from_parts<4, 2, 2, 0>) {
         uint32_t size;
         reader.read(size);
         reader.read_indexed_string(this->value);
         //
         // TODO: `size` should equal `sizeof(uint32_t) + this->value.size()`; maybe throw an error if it doesn't?
         //
      } else {
         reader.read_indexed_string(this->value);
      }
   }
}