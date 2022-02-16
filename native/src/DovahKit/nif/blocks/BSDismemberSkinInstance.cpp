#include "BSDismemberSkinInstance.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSDismemberSkinInstance::parse(file_reader& reader) {
      NiSkinInstance::parse(reader);
      //
      {
         auto& list = this->body_parts_per_partition;
         uint32_t count;
         reader.read(count);
         list.resize(count);
         reader.require_size(BodyPartList::serialized_size * count);
         reader.unchecked_read(list.data(), BodyPartList::serialized_size * count);
      }
   }
}