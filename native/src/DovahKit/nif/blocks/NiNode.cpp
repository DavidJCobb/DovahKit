#include "NiNode.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiNode::parse(file_reader& reader) {
      NiAVObject::parse(reader);
      //
      uint32_t count;
      //
      // children (generally nodes and geometry):
      //
      reader.read(count);
      this->children.resize(count);
      for (uint32_t i = 0; i < count; ++i)
         reader.read_ref(this->children[i]);
      //
      // effects:
      //
      reader.read(count);
      this->effects.resize(count);
      for (uint32_t i = 0; i < count; ++i)
         reader.read_ref(this->effects[i]);
   }
}