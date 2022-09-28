#include "bhkConvexVerticesShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkConvexVerticesShape::parse(file_reader& reader) {
      bhkConvexShape::parse(reader);
      reader.read(this->vertex_property);
      reader.read(this->normal_property);

      uint32_t count;

      reader.read(count);
      this->vertices.resize(count);
      reader.read_vector_contents(this->vertices);

      reader.read(count);
      this->normals.resize(count);
      reader.read_vector_contents(this->normals);
   }
}