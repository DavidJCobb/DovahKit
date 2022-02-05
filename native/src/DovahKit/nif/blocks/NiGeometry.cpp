#include "NiGeometry.h"
#include "../reader.h"
//
#include "BSShaderProperty.h"
#include "NiAlphaProperty.h"
#include "NiGeometryData.h"
#include "NiSkinInstance.h"

namespace nifDK::block_types {
   void NiGeometry::parse(file_reader& reader) {
      NiAVObject::parse(reader);
      reader.read_ref(this->data);
      if (reader.version() >= file_version::from_parts<3, 3, 0, 13>)
         reader.read_ref(this->skin);
      //
      if (reader.version() >= file_version::from_parts<10, 0, 1, 0> && reader.version() <= file_version::from_parts<20, 1, 0, 3>) {
         bool presence;
         reader.read(presence);
         if (presence) {
            reader.read_indexed_string(this->shader.name);
            reader.read(this->shader.extra);
         }
      } else if (reader.version() >= file_version::from_parts<20, 2, 0, 5>) {
         uint32_t count;
         reader.read(count);
         {
            auto& list = this->materials.list;
            list.resize(count);
            for (uint32_t i = 0; i < count; ++i)
               reader.read_indexed_string(list[i].name);
            for (uint32_t i = 0; i < count; ++i)
               reader.read(list[i].extra);
         }
         reader.read(this->materials.active);
      }
      if (reader.version() == file_version::from_parts<10, 2, 0, 0> && reader.user_version<1>() == 1) {
         reader.skip(1);
      }
      if (reader.version() == file_version::from_parts<10, 4, 0, 1>) {
         reader.skip(4);
      }
      if (reader.version() >= file_version::from_parts<20, 2, 0, 7> && reader.user_version<1>() == 12) {
         reader.read(this->materials.needs_update);
         //
         reader.read_ref(this->properties.shader);
         reader.read_ref(this->properties.alpha);
      }
   }
}