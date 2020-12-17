#include "map_marker.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result map_marker::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
      if (subrecord.signature() != signature)
         return load_result::unrecognized;
      return load_result::requires_record;
   }
   bool map_marker::load(tes_record_reader& record, load_interface_t& intfc) {
      if (record.peek_next_subrecord_type() == 'FNAM') { // the game considers FNAM optional
         auto& subrecord = record.next_subrecord();
         subrecord.read(this->flags);
      }
      if (record.peek_next_subrecord_type() == 'FULL') {
         auto& subrecord = record.next_subrecord();
         subrecord.to_string(this->name);
         if (record.peek_next_subrecord_type() == 'TNAM') { // the game only checks for TNAM after FULL
            auto& subrecord = record.next_subrecord();
            subrecord.read(this->type);
         }
      }
      return true;
   }
   void map_marker::save(tes_record_writer& record, save_interface_t& intfc) {
      auto& XMRK = record.open_next_subrecord(signature);
      XMRK.close();
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->flags);
      FNAM.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& TNAM = record.open_next_subrecord('TNAM');
      TNAM.write(this->type);
      TNAM.close();
   }
   basic_extra_data* map_marker::clone(form_stub& clone_owner) const noexcept {
      auto* clone = new map_marker;
      clone->flags = this->flags;
      clone->name  = this->name;
      clone->type  = this->type;
      return clone;
   }
}