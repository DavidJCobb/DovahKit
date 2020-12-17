#include "ragdoll_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result ragdoll_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      switch (subrecord.signature()) {
         case signature_base:
            this->has_rgd = true;
            this->data_rgd.resize(subrecord.size());
            subrecord.read(this->data_rgd.data(), subrecord.size());
            break;
         case signature_biped:
            this->has_rgb = true;
            subrecord.read(this->data_rgb.data(), this->data_rgb.size());
            break;
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   void ragdoll_data::save(tes_record_writer& record, save_interface_t& intfc) {
      if (this->has_rgd) {
         auto& subrecord = record.open_next_subrecord(signature_base);
         subrecord.write(this->data_rgd.data(), this->data_rgd.size());
         subrecord.close();
      }
      if (this->has_rgb) {
         auto& subrecord = record.open_next_subrecord(signature_biped);
         subrecord.write(this->data_rgb.data(), this->data_rgb.size());
         subrecord.close();
      }
   }
   basic_extra_data* ragdoll_data::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new ragdoll_data;
      clone->has_rgd = this->has_rgd;
      clone->has_rgb = this->has_rgb;
      clone->data_rgd = this->data_rgd;
      clone->data_rgb = this->data_rgb;
      return clone;
   }
}