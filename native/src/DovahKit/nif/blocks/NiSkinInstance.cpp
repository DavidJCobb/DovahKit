#include "NiSkinInstance.h"
#include "../reader.h"

#include "NiNode.h"
#include "NiSkinData.h"
#include "NiSkinPartition.h"

namespace nifDK::block_types {
   void NiSkinInstance::parse(file_reader& reader) {
      reader.read_ref(this->data);
      reader.read_ref(this->partition);
      reader.read_ref(this->armature.root);
      //
      uint32_t count;
      reader.read(count);
      this->armature.bones.resize(count);
      for (uint32_t i = 0; i < count; ++i)
         reader.read_ref(this->armature.bones[i]);
   }
}