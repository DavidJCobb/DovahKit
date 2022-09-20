#include "BSMultiBoundNode.h"
#include "BSMultiBound.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSMultiBoundNode::parse(file_reader& reader) {
      NiNode::parse(reader);
      //
      reader.read_ref(this->multibound);
      if (reader.user_version<2>() >= 83) {
         reader.read(this->culling_mode);
      }
   }
}