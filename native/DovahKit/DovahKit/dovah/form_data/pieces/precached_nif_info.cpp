#include "./precached_nif_info.h"
#include "dovah/tes/reading/subrecord_view.h"

namespace dovah::form_data::pieces {
   bool precached_nif_info::file_id::stream(tes::subrecord_view& subrecord) {
      return subrecord.read(
         this->file_hash,
         lu::bytestreams::apply_modifiers<lu::bytestreams::opposite_endianness>::to(this->extension),
         this->folder_hash
      );
   }
   void precached_nif_info::file_id::unchecked_stream(tes::subrecord_view& subrecord) {
      subrecord.unchecked_read(
         this->file_hash,
         lu::bytestreams::apply_modifiers<lu::bytestreams::opposite_endianness>::to(this->extension),
         this->folder_hash
      );
   }
}