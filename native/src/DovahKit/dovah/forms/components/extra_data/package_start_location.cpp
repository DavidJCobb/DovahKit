#include "package_start_location.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result package_start_location::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->unk00);
      subrecord.read(this->unk04);
      subrecord.read(this->unk08);
      subrecord.read(this->unk0C);
      subrecord.read(this->unk10);
      return load_result::succeeded;
   }
   void package_start_location::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->unk00);
      subrecord.write(this->unk04);
      subrecord.write(this->unk08);
      subrecord.write(this->unk0C);
      subrecord.write(this->unk10);
      subrecord.close();
   }
   basic_extra_data* package_start_location::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new package_start_location;
      clone->unk00 = this->unk00;
      clone->unk04 = this->unk04;
      clone->unk08 = this->unk08;
      clone->unk0C = this->unk0C;
      clone->unk10 = this->unk10;
      return clone;
   }
}