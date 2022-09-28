#include "bhkNiTriStripsShape.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkNiTriStripsShape::parse(file_reader& reader) {
      uint32_t count;

      reader.read(this->material);
      reader.read(this->radius);
      reader.read(this->pad08);
      reader.read(this->max_size);
      reader.read(this->size);
      reader.read(this->e_size);
      reader.read(this->grow_by);
      reader.read(this->scale);
      
      reader.read(count);
      this->strips.resize(count);
      for (auto& item : this->strips)
         reader.read_ref(item.data);
      for (auto& item : this->strips)
         reader.read(item.filter);
   }
}