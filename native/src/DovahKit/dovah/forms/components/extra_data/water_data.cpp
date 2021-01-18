#include "water_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result water_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      auto s = subrecord.signature();
      if (s == signature_base) {
         subrecord.read(this->count);
         return load_result::requires_record;
      }
      return load_result::unrecognized;
   }
   bool water_data::load(tes_record_reader& record, load_interface_t& intfc) {
      auto signature = record.peek_next_subrecord_type();
      if (signature == signature_vel) {
         auto& subrecord = record.next_subrecord();
         this->data.resize(this->count);
         for (auto& v : this->data) {
            subrecord.read(v.velocity.x);
            subrecord.read(v.velocity.y);
            subrecord.read(v.velocity.z);
            subrecord.read(v.unk0C);
         }
      } else {
         //
         // Skyrim doesn't double-check the signature, but we're going to.
         //
         return false;
      }
      return true;
   }
   void water_data::save(tes_record_writer& record, save_interface_t& intfc) {
      this->count = this->data.size();
      //
      auto& XWCN = record.open_next_subrecord(signature_base);
      XWCN.write(this->count);
      XWCN.close();
      auto& XWCU = record.open_next_subrecord(signature_vel);
      for (auto& v : this->data) {
         XWCU.write(v.velocity.x);
         XWCU.write(v.velocity.y);
         XWCU.write(v.velocity.z);
         XWCU.write(v.unk0C);
      }
      XWCU.close();
   }
   basic_extra_data* water_data::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new water_data;
      //
      size_t size = this->data.size();
      clone->data.resize(size);
      for (size_t i = 0; i < size; ++i) {
         clone->data[i] = this->data[i];
      }
      clone->count = size;
      //
      return clone;
   }
}