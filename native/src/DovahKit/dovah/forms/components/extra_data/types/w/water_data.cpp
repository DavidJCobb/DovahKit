#include "./water_data.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

#include "../../../../../notices/form_load_warnings/by_form_component/extra_data/water_data_swallowed_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::extra_data;
   }
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result water_data::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() == signature_base)
         return subrecord_load_result::requires_record;
      return subrecord_load_result::unrecognized;
   }
   /*virtual*/ record_load_result water_data::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      uint32_t count = 0;
      {
         auto& subrecord = record.get_current_subrecord();
         assert(subrecord.signature() == signature_base && "This function should only have been called after the other `load` overload verified that we were in an XWCN subrecord.");
         subrecord.read(count);
      }

      auto& subrecord = record.next_subrecord();
      if (subrecord.signature() != signature_vel) {
         specific_load_warnings::water_data_swallowed_subrecord notice(
            intfc.target_stub,
            subrecord.signature()
         );
         intfc.log_load_warning(notice);
      }
      this->data.resize(count);
      for (auto& v : this->data) {
         subrecord.read(v.velocity.x);
         subrecord.read(v.velocity.y);
         subrecord.read(v.velocity.z);
         subrecord.read(v.unk0C);
      }
      return record_load_result::complete;
   }
   /*virtual*/ void water_data::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& XWCN = record.open_next_subrecord(signature_base);
      XWCN.write((uint32_t)this->data.size());
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
   
   /*static*/ void water_data::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
   }
   /*virtual*/ void water_data::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }
   /*virtual*/ void water_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }
   
   /*virtual*/ extra_data* water_data::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new water_data;
      //
      size_t size = this->data.size();
      clone->data.resize(size);
      for (size_t i = 0; i < size; ++i) {
         clone->data[i] = this->data[i];
      }
      //
      return clone;
   }
}