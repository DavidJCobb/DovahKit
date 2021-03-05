#include "activate_parent_data.h"
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::components::extra {
   extra_data_load_result activate_parent_data::load(tes_subrecord_reader& subrecord, load_interface_t& intfc) {
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
            {
               auto& entry = this->parents.emplace_back();
               subrecord.read(entry.ref);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_not_object_reference(subrecord.signature(), intfc.target_stub, entry.ref)
               );
               subrecord.read(entry.delay);
            }
            break;
         default:
            return load_result::unrecognized;
      }
      return load_result::succeeded;
   }
   void activate_parent_data::save(tes_record_writer& record, save_interface_t& intfc) {
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
   //
   /*static*/ void activate_parent_data::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == signature_parent) {
         form_id_t formID;
         if (subrecord.read(formID) && formID)
            uib.add_outbound_reference(formID);
      }
   }
   basic_extra_data* activate_parent_data::clone(loaded_forms::Form& clone_owner) const noexcept {
      auto* clone = new activate_parent_data;
      clone->flags = this->flags;
      //
      size_t size = this->parents.size();
      clone->parents.resize(size);
      for (size_t i = 0; i < size; ++i) {
         clone->parents[i].ref.set(clone_owner, this->parents[i].ref);
         clone->parents[i].delay = this->parents[i].delay;
      }
      //
      return clone;
   }
   void activate_parent_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) {
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
}