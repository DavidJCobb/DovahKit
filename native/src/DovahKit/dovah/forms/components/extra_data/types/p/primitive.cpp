#include "./primitive.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

#include "../../../../../notices/form_load_warnings/by_form_component/extra_data/primitive_is_zero_size.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::extra_data;
   }
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result primitive::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      subrecord.read(this->halfwidths.x);
      subrecord.read(this->halfwidths.y);
      subrecord.read(this->halfwidths.z);
      this->color.load(subrecord);
      subrecord.read(this->shape);

      if (this->halfwidths.length() < 0.001F) { // same warning threshold as the CK
         specific_load_warnings::primitive_is_zero_size notice(intfc.target_stub);
         intfc.log_load_warning(notice);
      }

      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result primitive::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void primitive::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->halfwidths.x);
      subrecord.write(this->halfwidths.y);
      subrecord.write(this->halfwidths.z);
      this->color.save(subrecord);
      subrecord.write(this->shape);
      subrecord.close();
   }
   
   /*static*/ void primitive::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void primitive::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void primitive::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* primitive::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new primitive;
      clone->halfwidths = this->halfwidths;
      clone->color  = this->color;
      clone->shape  = this->shape;
      return clone;
   }
}