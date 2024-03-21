#include "BSValueNode.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSValueNode::parse(file_reader& reader) {
      NiNode::parse(reader);
      //
      reader.read(this->value);
      reader.read(this->flags);
   }
}