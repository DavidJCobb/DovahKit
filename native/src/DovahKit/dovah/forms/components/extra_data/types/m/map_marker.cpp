#include "./map_marker.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result map_marker::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      return subrecord_load_result::requires_record;
   }
   /*virtual*/ record_load_result map_marker::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      if (record.peek_next_subrecord_type() == 'FNAM') { // the game considers FNAM optional
         auto& subrecord = record.next_subrecord();
         subrecord.read(this->flags);
      }
      if (record.peek_next_subrecord_type() == 'FULL') {
         auto& subrecord = record.next_subrecord();
         subrecord.read(this->name);
         if (record.peek_next_subrecord_type() == 'TNAM') { // the game only checks for TNAM after FULL
            auto& subrecord = record.next_subrecord();
            subrecord.read(this->type);
         }
      }
      return record_load_result::complete;
   }
   /*virtual*/ void map_marker::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
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
   
   /*static*/ void map_marker::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void map_marker::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void map_marker::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* map_marker::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new map_marker;
      clone->flags = this->flags;
      clone->name = this->name;
      clone->type = this->type;
      return clone;
   }
}