#include "./cell_region_list.h"
#include "../../../../_common_cpp.h"
#include "../../use_info_state.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result cell_region_list::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      auto size = subrecord.size() / 4;
      this->regions.resize(size);
      for (size_t i = 0; i < size; ++i) {
         subrecord.read(this->regions[i]);
         intfc.warn_if_ref_is_wrong_type(this->regions[i], form_type::region, subrecord, { .nth_reference = i });
      }
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result cell_region_list::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void cell_region_list::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      if (this->regions.empty())
         return;
      auto& subrecord = record.open_next_subrecord(signature);
      for (auto id : this->regions)
         subrecord.write(id);
      subrecord.close();
   }
   
   /*static*/ void cell_region_list::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature) {
         uis.cell_region_list.clear();
         auto count = subrecord.size() / 4;
         for (size_t i = 0; i < count; ++i) {
            form_id_t formID;
            if (subrecord.read(formID))
               uis.cell_region_list.push_back(formID);
         }
      }
   }
   /*virtual*/ void cell_region_list::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      clear_form_reference_list(this->regions, my_owner);
   }
   /*virtual*/ void cell_region_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      remove_form_from_reference_list(this->regions, target, my_owner);
   }
   
   /*virtual*/ extra_data* cell_region_list::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new cell_region_list;
      copy_form_reference_list(clone_owner, clone->regions, this->regions);
      return clone;
   }
}