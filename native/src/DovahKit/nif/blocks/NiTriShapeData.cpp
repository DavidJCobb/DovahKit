#include "NiTriShapeData.h"
#include "../reader.h"
#include "../notice_code_list.h"

namespace nifDK::block_types {
   void NiTriShapeData::parse(file_reader& reader) {
      NiTriBasedGeomData::parse(reader);
      //
      uint16_t count;
      uint32_t check;
      reader.read(count); // Num Triangles
      reader.read(check); // Num Triangle Points
      if (check != (decltype(check))count * 3) {
         reader.throw_error(notice_code::inconsistent_triangle_counts);
      }
      bool presence = reader.version() < file_version::from_parts<10, 1, 0, 0>;
      if (!presence)
         reader.read(presence);
      if (presence) {
         this->triangles.resize(count);
         reader.read_vector_contents(this->triangles);
      }
      if (reader.version() >= file_version::from_parts<3, 1, 0, 0>) {
         reader.read(count);
         this->match_groups.resize(count);
         for (auto& mg : this->match_groups) {
            uint16_t count;
            reader.read(count);
            mg.vertex_indices.resize(count);
            reader.read_vector_contents(mg.vertex_indices);
         }
      }
   }
}