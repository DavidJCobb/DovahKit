#include "activate_parent_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result activate_parent_data::load(tes_subrecord_reader& subrecord) {
      switch (subrecord.signature()) {
         case signature_flags:
            if (subrecord.size() == 4) {
               uint32_t data;
               subrecord.read(data);
               this->flags = data;
            } else {
               subrecord.read(this->flags);
            }
            break;
         case signature_parent:
            {
               auto& entry = this->parents.emplace_back();
               subrecord.read(entry.ref);
               subrecord.read(entry.delay);
            }
            break;
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   void activate_parent_data::save(tes_record_writer& record) {
      auto& XAPD = record.open_next_subrecord(signature_flags);
      XAPD.write(this->flags);
      XAPD.close();
      for (auto& entry : this->parents) {
         auto& subrecord = record.open_next_subrecord(signature_parent);
         subrecord.write(entry.ref);
         subrecord.write(entry.delay);
         subrecord.close();
      }
   }
}