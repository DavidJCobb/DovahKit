#include "distant_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result distant_data::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->unk00);
      subrecord.read(this->unk04);
      subrecord.read(this->unk08);
      return load_result::succeeded;
   }
   void distant_data::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->unk00);
      subrecord.write(this->unk04);
      subrecord.write(this->unk08);
      subrecord.close();
   }
}