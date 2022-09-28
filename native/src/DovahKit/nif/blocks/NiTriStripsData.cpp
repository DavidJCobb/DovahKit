#include "NiTriStripsData.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiTriStripsData::parse(file_reader& reader) {
      NiTriBasedGeomData::parse(reader);
      
      uint16_t strip_count;
      uint16_t point_count = 0;

      reader.read(strip_count);
      this->strip_lengths.resize(strip_count);
      for (auto& item : this->strip_lengths) {
         reader.read(item);
         point_count += item;
      }

      bool has_points = true;
      if (reader.version() >= file_version::from_parts<10, 0, 1, 3>) {
         reader.read(has_points);
      }
      if (has_points) {
         this->points.resize(point_count);
         reader.read_vector_contents(this->points);
      }
   }
}