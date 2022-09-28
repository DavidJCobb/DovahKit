#include "bhkListShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkListShape::parse(file_reader& reader) {
      uint32_t count;

      reader.read(count);
      this->contents.resize(count);
      for (auto& item : this->contents)
         reader.read_ref(item);

      reader.read(this->material);
      reader.read(this->child_shape_property);
      reader.read(this->child_filter_property);

      reader.read(count);
      this->unknown.resize(count);
      reader.read_vector_contents(this->unknown);
   }
}