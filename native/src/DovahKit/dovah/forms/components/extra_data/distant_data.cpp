#include "distant_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result distant_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->unk00);
      subrecord.read(this->unk04);
      subrecord.read(this->unk08);
      return load_result::succeeded;
   }
   void distant_data::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->unk00);
      subrecord.write(this->unk04);
      subrecord.write(this->unk08);
      subrecord.close();
   }
   basic_extra_data* distant_data::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new distant_data;
      clone->unk00 = this->unk00;
      clone->unk04 = this->unk04;
      clone->unk08 = this->unk08;
      return clone;
   }
}