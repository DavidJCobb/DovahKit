#include "bhkWorldObject.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkWorldObject::parse(file_reader& reader) {
      reader.read_ref(this->shape);
      if (reader.version() <= file_version::from_parts<10, 0, 1, 2>) {
         reader.read(this->unk04);
      }
      reader.read(this->filter);
      reader.read(this->pad0C);
      reader.read(this->broad_phase);
      reader.read(this->pad11);
      reader.read(this->cinfo);
   }
}