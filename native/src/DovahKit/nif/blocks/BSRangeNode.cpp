#include "BSRangeNode.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSRangeNode::parse(file_reader& reader) {
      NiNode::parse(reader);
      //
      reader.read(this->range.min);
      reader.read(this->range.max);
      reader.read(this->range.current);
   }
}