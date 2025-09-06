#include "BSFurnitureMarkerNode.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSFurnitureMarkerNode::parse(file_reader& reader) {
      NiExtraData::parse(reader);

      uint32_t count = 0;
      reader.read(count);
      this->markers.reserve(count);
      for (uint32_t i = 0; i < count; ++i) {
         auto& item = this->markers.emplace_back();
         reader.read(item.position);
         reader.read(item.yaw);

         uint16_t anim_type = 0;
         reader.read(anim_type);
         item.animation_types.overwrite_with_raw_integer(anim_type);

         uint16_t entry_points = 0;
         reader.read(entry_points);
         item.entry_points.overwrite_with_raw_integer(entry_points);
      }
   }
}