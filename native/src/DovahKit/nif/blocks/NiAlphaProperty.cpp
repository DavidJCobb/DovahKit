#include "NiAlphaProperty.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiAlphaProperty::parse(file_reader& reader) {
      NiProperty::parse(reader);
      //
      uint16_t data;
      if (reader.version() <= file_version::from_parts<2, 3, 0, 0>) {
         uint32_t dummy;
         reader.read(dummy);
         //
         // No one knows how this works, and I guess it's in a NIF version too old for anyone 
         // to actually care about.
         //
         return;
      }
      reader.read(data);
      //
      this->blending.enabled     = (data & (1 << 0)) != 0;
      this->blending.source      = (blend_mode)((data >> 1) & 0b1111);
      this->blending.destination = (blend_mode)((data >> 5) & 0b1111);
      //
      reader.read(this->testing.threshold);
      this->testing.enabled      = (data & (1 << 9)) != 0;
      this->testing.mode         = (test_mode)((data >> 10) & 0b111);
      this->testing.no_sorter    = (data & (1 << 13)) != 0;
      this->testing.configurable = (data & (1 << 15)) != 0;
   }
}