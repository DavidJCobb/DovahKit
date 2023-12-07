#include "./attachment_header.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::papyrus {
   bool attachment_header::load(tes_subrecord_reader& subrecord) {
      if (subrecord.read(this->version))
         if (subrecord.read(this->object_format))
            return true;
      return false;
   }
   void attachment_header::skip(tes_subrecord_reader& subrecord) {
      subrecord.skip_bytes(sizeof(version) + sizeof(object_format));
   }
   void attachment_header::save(tes_subrecord_writer& subrecord) const {
      subrecord.write(this->version);
      subrecord.write(this->object_format);
   }
}