#include "NiSkinData.h"
#include "../reader.h"
#include "../types/Float16.h"

#include "NiSkinPartition.h"

namespace nifDK::block_types {
   void NiSkinData::bone::vertex::unchecked_read(file_reader& reader) {
      reader.unchecked_read(this->index);
      reader.unchecked_read(this->weight);
   }

   void NiSkinData::bone::parse(file_reader& reader, uint8_t presence) {
      reader.read(this->transform);
      reader.read(this->bound);
      if (reader.version() == file_version::from_parts<20, 3, 0, 9>) {
         switch (reader.user_version<1>()) {
            case 0x20000:
            case 0x30000:
               reader.skip(sizeof(uint16_t) * 13); // 13 unknown shorts
               break;
         }
      }
      //
      uint16_t count;
      reader.read(count);
      this->vertex_weights.resize(count);
      if (reader.version() >= file_version::from_parts<20, 3, 1, 1> && presence == 15) {
         reader.require_size(count * vertex::serialized_size_half);
         for (auto& w : this->vertex_weights) {
            reader.unchecked_read(w.index);
            Float16 f;
            reader.unchecked_read(f);
            w.weight = f;
         }
      } else {
         reader.require_size(count * vertex::serialized_size);
         for (size_t i = 0; i < count; ++i)
            this->vertex_weights[i].unchecked_read(reader);
      }
   }

   void NiSkinData::parse(file_reader& reader) {
      reader.read(this->transform);
      uint32_t count;
      reader.read(count);
      if (reader.version() >= file_version::from_parts<4, 0, 0, 2> && reader.version() <= file_version::from_parts<10, 1, 0, 0>)
         reader.read_ref(this->partition);
      if (reader.version() >= file_version::from_parts<4, 2, 1, 0>) {
         uint8_t presence;
         reader.read(presence);
         if (presence) {
            this->bones.resize(count);
            for (auto& b : this->bones)
               b.parse(reader, presence);
         }
      }
   }
}