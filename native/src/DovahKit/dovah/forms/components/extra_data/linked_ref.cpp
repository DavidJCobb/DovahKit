#include "linked_ref.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result linked_ref::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      if (subrecord.size() >= 8)
         subrecord.read(this->keyword);
      subrecord.read(this->ref);
      return load_result::succeeded;
   }
   void linked_ref::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      if (this->keyword)
         subrecord.write(this->keyword);
      subrecord.write(this->ref);
      subrecord.close();
   }
}