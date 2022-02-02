#include "NiGeometry.h"
#include "../reader.h"

namespace nifDK::block_types {
   void NiGeometry::parse(file_reader& reader) {
      NiAVObject::parse(reader);
      reader.read_ref(this->data);
      reader.read_ref(this->skin);
      //
      uint32_t count;
      //
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
      // "material" needs update bool -- 20.2.0.7+
      // 
      reader.read_ref(this->properties.shader);
      reader.read_ref(this->properties.alpha);
   }
}