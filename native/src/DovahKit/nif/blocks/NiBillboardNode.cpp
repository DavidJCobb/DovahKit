#include "NiBillboardNode.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiBillboardNode::parse(file_reader& reader) {
      NiNode::parse(reader);
      //
      if (reader.version() >= file_version::from_parts<10, 1, 0, 0>) {
         reader.read(this->mode);
      } else {
         this->mode = (billboard_mode)((this->flags >> 5) & 0b11);
      }
   }
}