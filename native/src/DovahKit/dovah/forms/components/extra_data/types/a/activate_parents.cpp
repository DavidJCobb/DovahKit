#include "./activate_parents.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result activate_parents::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      switch (subrecord.signature()) {
         case signature_flags:
            if (subrecord.size() == 4) {
               uint32_t data;
               subrecord.read(data);
               this->flags = data;
            } else {
               subrecord.read(this->flags);
            }
            break;
         case signature_parent:
         case signature_parent_legacy:
            {
               auto& entry = this->parents.emplace_back();
               subrecord.read(entry.ref);
               intfc.warn_if_ref_is_wrong_type(entry.ref, form_type::reference, subrecord, { .nth_reference = this->parents.size() - 1 });
               subrecord.read(entry.delay);
            }
            break;
         default:
            return subrecord_load_result::unrecognized;
      }
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result activate_parents::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void activate_parents::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& XAPD = record.open_next_subrecord(signature_flags);
      XAPD.write(this->flags);
      XAPD.close();
      for (auto& entry : this->parents) {
         auto& subrecord = record.open_next_subrecord(signature_parent);
         subrecord.write(entry.ref);
         subrecord.write(entry.delay);
         subrecord.close();
      }
   }
   
   /*static*/ void activate_parents::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_parent) {
         form_id_t formID;
         if (subrecord.read(formID) && formID)
            uib.add_outbound_reference(formID);
      }
   }
   /*virtual*/ void activate_parents::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      for (auto& item : this->parents) {
         item.ref.set(my_owner, nullptr);
      }
      this->parents.clear();
   }
   /*virtual*/ void activate_parents::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      auto& list = this->parents;
      for (auto& entry : list)
         entry.ref.clear_if(my_owner, target);
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [](parent& entry) {
               return entry.ref == nullptr;
            }
         ),
         list.end()
      );
   }
   
   /*virtual*/ extra_data* activate_parents::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new activate_parents;
      clone->flags = this->flags;
      
      size_t size = this->parents.size();
      clone->parents.resize(size);
      for (size_t i = 0; i < size; ++i) {
         clone->parents[i].ref.set(clone_owner, this->parents[i].ref);
         clone->parents[i].delay = this->parents[i].delay;
      }
      
      return clone;
   }
}