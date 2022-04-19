#include "NiNode.h"
#include "../detailed_notice.h"
#include "../notice_code_list.h"
#include "../reader.h"

#include "NiDynamicEffect.h"

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
      for (uint32_t i = 0; i < count; ++i) {
         reader.read_ref(this->children[i]);
         //
         if (auto* child = this->children[i]) {
            if (child->parent) {
               reader.raise_error(detailed_notice{
                  .code     = notice_code::object_has_multiple_parents,
                  .relevant = {
                     .block_indices = { reader.index_of_block(child) },
                  },
               });
            }
            child->parent = this;
         }
      }
      //
      // effects:
      //
      reader.read(count);
      this->effects.resize(count);
      for (uint32_t i = 0; i < count; ++i)
         reader.read_ref(this->effects[i]);
   }

   size_t NiNode::index_of_child(const NiAVObject* c) const {
      auto& list = this->children;
      auto  size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (list[i] == c)
            return i;
      return -1;
   }
}