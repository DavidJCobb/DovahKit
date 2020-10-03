#include "light.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result light::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      subrecord.read(this->fov);
      subrecord.read(this->fade);
      subrecord.read(this->unk08);
      subrecord.read(this->shadow_depth_bias);
      subrecord.read(this->unk10); // optional
      return load_result::succeeded;
   }
   void light::save(tes_record_writer& record) {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->fov);
      subrecord.write(this->fade);
      subrecord.write(this->unk08);
      subrecord.write(this->shadow_depth_bias);
      subrecord.write(this->unk10);
      subrecord.close();
   }
   basic_extra_data* light::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new light;
      clone->fov   = this->fov;
      clone->fade  = this->fade;
      clone->unk08 = this->unk08;
      clone->shadow_depth_bias = this->shadow_depth_bias;
      clone->unk10 = this->unk10;
      return clone;
   }
}