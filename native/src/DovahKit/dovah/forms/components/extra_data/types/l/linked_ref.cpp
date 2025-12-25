#include "./linked_ref.h"
#include "../../../../_common_cpp.h"

namespace {
   using extra_data            = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result    = extra_data::record_load_result;
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result linked_ref::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      auto& item = this->links.emplace_back();
      if (subrecord.size() >= 8) {
         subrecord.read(item.keyword);
         intfc.warn_if_ref_is_wrong_type(item.keyword, form_type::keyword, subrecord.signature());
      }
      subrecord.read(item.ref);
      intfc.warn_if_ref_is_wrong_type(item.ref, form_type::reference, subrecord.signature());
      return subrecord_load_result::succeeded;
   }
   /*virtual*/ record_load_result linked_ref::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }
   /*virtual*/ void linked_ref::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      for (auto& link : this->links) {
         if (!link.ref)
            continue;
         auto& subrecord = record.open_next_subrecord(signature);
         if (link.keyword)
            subrecord.write(link.keyword);
         subrecord.write(link.ref);
         subrecord.close();
      }
   }
   
   /*static*/ void linked_ref::generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& uis) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.is_in_bounds(8)) {
         subrecord.read(uis.form_ids.by_name.linked_ref.keyword);
         subrecord.read(uis.form_ids.by_name.linked_ref.ref);
         return;
      }
      subrecord.read(uis.form_ids.by_name.linked_ref.ref);
   }
   /*virtual*/ void linked_ref::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      for (auto& link : this->links) {
         link.keyword.set(my_owner, nullptr);
         link.ref.set(my_owner, nullptr);
      }
      this->links.clear();
   }
   /*virtual*/ void linked_ref::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      bool any_removed = false;
      for (auto& link : this->links) {
         link.keyword.clear_if(my_owner, target);
         if (link.ref) {
            link.ref.clear_if(my_owner, target);
            if (!link.ref) {
               any_removed = true;
               link.keyword.set(my_owner, nullptr);
            }
         }
      }
      if (any_removed) {
         std::erase_if(this->links, [](auto& link) { return !link.ref; });
      }
   }
   
   /*virtual*/ extra_data* linked_ref::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* clone = new linked_ref;

      size_t size = this->links.size();
      clone->links.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src = this->links[i];
         auto& dst = clone->links[i];
         dst.keyword.set(clone_owner, src.keyword);
         dst.ref.set(clone_owner, src.ref);
      }

      return clone;
   }
}