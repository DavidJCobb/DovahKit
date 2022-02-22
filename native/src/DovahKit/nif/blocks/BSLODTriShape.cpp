#include "BSLODTriShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void BSLODTriShape::parse(file_reader& reader) {
      NiTriBasedGeom::parse(reader);
      //
      reader.read(this->lod_sizes);
   }
}