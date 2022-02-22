#include "NiSwitchNode.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiSwitchNode::parse(file_reader& reader) {
      NiNode::parse(reader);
      //
      reader.read(this->flags);
      reader.read(this->current_child_index);
   }

   NiAVObject* NiSwitchNode::current_child() const {
      auto& list = this->children;
      if (list.empty())
         return nullptr;
      if (this->current_child_index >= list.size())
         return list[0];
      return list[this->current_child_index];
   }
}