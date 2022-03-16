#include "BSOrderedNode.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSOrderedNode::parse(file_reader& reader) {
      NiNode::parse(reader);
      //
      reader.read(this->alpha_sort_bound);
      reader.read(this->static_bound);
   }
}