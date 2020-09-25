#include "lock.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result lock::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->level);
      subrecord.read(this->pad01);
      subrecord.read(this->key);
      subrecord.read(this->flags);
      subrecord.read(this->pad09);
      subrecord.read(this->unk0C);
      subrecord.read(this->unk10);
      return load_result::succeeded;
   }
   void lock::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->level);
      subrecord.write(this->pad01);
      subrecord.write(this->key);
      subrecord.write(this->flags);
      subrecord.write(this->pad09);
      subrecord.write(this->unk0C);
      subrecord.write(this->unk10);
      subrecord.close();
   }
}