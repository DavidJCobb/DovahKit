#include "extra_data.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   void ragdoll_biped_data::load(tes_subrecord_reader& subrecord) {
      if (subrecord.is_in_bounds(this->bytes.size())) {
         for (auto& byte : this->bytes)
            subrecord.unchecked_read(byte);
      }
   }
   void ragdoll_biped_data::save(tes_subrecord_writer& subrecord) {
      for (auto& byte : this->bytes)
         subrecord.write(byte);
   }
}