#include "./portal.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result portal::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->width);
      subrecord.read(this->height);
      subrecord.read(this->position.x);
      subrecord.read(this->position.y);
      subrecord.read(this->position.z);
      subrecord.read(this->rotation.a);
      subrecord.read(this->rotation.b);
      subrecord.read(this->rotation.c);
      subrecord.read(this->rotation.d);
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result portal::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void portal::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->width);
      subrecord.write(this->height);
      subrecord.write(this->position.x);
      subrecord.write(this->position.y);
      subrecord.write(this->position.z);
      subrecord.write(this->rotation.a);
      subrecord.write(this->rotation.b);
      subrecord.write(this->rotation.c);
      subrecord.write(this->rotation.d);
      subrecord.close();
   }
   
   /*static*/ void portal::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void portal::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void portal::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* portal::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new portal;
      clone->width    = this->width;
      clone->height   = this->height;
      clone->position = this->position;
      clone->rotation = this->rotation;
      return clone;
   }
}