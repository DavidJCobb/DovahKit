#include "activate_ref.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result activate_ref::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->unk00);
      subrecord.read(this->unk04);
      subrecord.read(this->unk08);
      subrecord.read(this->pad09);
      return load_result::succeeded;
   }
   void activate_ref::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->unk00);
      subrecord.write(this->unk04);
      subrecord.write(this->unk08);
      subrecord.write(this->pad09);
      subrecord.close();
   }
   basic_extra_data* activate_ref::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new activate_ref;
      clone->unk00 = this->unk00;
      clone->unk04 = this->unk04;
      clone->unk08 = this->unk08;
      clone->pad09[0] = this->pad09[0];
      clone->pad09[1] = this->pad09[1];
      clone->pad09[2] = this->pad09[2];
      return clone;
   }
}