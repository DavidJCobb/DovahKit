#include "BSVertexDesc.h"
#include "../reader.h"

namespace nifDK {
   void BSVertexDesc::read(file_reader& reader) {
      reader.require_size(8);
      reader.unchecked_read(this->unk00);
      reader.unchecked_read(this->flags);
      reader.unchecked_read(this->unk07);
   }
}