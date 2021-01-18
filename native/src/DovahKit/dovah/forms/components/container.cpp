#include "container.h"
#include "../_common_cpp.h"
#include "../../notice_code_list.h"

namespace dovah::loaded_forms::components {
   void container_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      if (subrecord.signature() == 'CNTO') { // CoNTainer Object
         auto& entry = this->entries.emplace_back();
         subrecord.unchecked_read(entry.item);
         subrecord.unchecked_read(entry.count);
         return;
      }
      if (subrecord.signature() == 'COED') { // Container Object Extra Data
         //
         // There's a formID followed by an integer/formID union whose type depends on the form 
         // type of the formID preceding it. Fortunately, we load all form stubs before we load 
         // any one form, so we can identify the type of the owner form from here.
         //
         if (!this->entries.size())
            return;
         auto& entry = this->entries.back();
         if (subrecord.read(entry.ownership.owner)) {
            auto ownerStub = subrecord.lookup_form_by_id(entry.ownership.owner);
            intfc.log_load_warning(
               detailed_notice::warn_if_wrong_type(subrecord.signature(), { form_type::actor_base, form_type::faction }, intfc.target_stub, entry.ownership.owner)
            );
            if (ownerStub && ownerStub->formType == form_type::actor_base) {
               subrecord.unchecked_read(entry.ownership.global);
               intfc.log_load_warning(
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::global, intfc.target_stub, entry.ownership.global)
               );
            } else {
               subrecord.unchecked_read(entry.ownership.faction_rank);
               //
               if (ownerStub && ownerStub->formType != form_type::faction) {
                  detailed_notice warning;
                  warning.code = notice_code::container_item_has_bad_owner_form_type;
                  warning.set_cause_form(intfc.target_stub);
                  warning.set_cause_subrecord(subrecord.signature());
                  warning.add_relevant_form(*ownerStub);
                  intfc.log_load_warning(warning);
               }
            }
            entry.condition.present = true;
            subrecord.unchecked_read(entry.condition.value);
         }
         return;
      }
      if (subrecord.signature() == 'COCT') {
         uint32_t count;
         if (subrecord.read(count))
            this->entries.reserve(count);
         return;
      }
      #if _DEBUG
         __debugbreak();
      #else
         assert(false && "ContainerData::load should only be called for COCT, CNTO, and COED subrecords!");
      #endif
   }
   bool container_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (this->entries.empty())
         return true;
      auto& COCT = record.open_next_subrecord('COCT');
      COCT.write(uint32_t(this->entries.size()));
      COCT.close();
      for (auto& entry : this->entries) {
         auto& CNTO = record.open_next_subrecord('CNTO');
         CNTO.write(entry.item);
         CNTO.write(entry.count);
         CNTO.close();
         if (entry.ownership.owner || entry.condition.present) {
            auto& COED = record.open_next_subrecord('COED');
            COED.write(entry.ownership.owner);
            if (auto* stub = entry.ownership.owner.get_form_stub()) {
               if (stub->formType == form_type::actor_base) {
                  COED.write(entry.ownership.global);
               } else {
                  COED.write(entry.ownership.faction_rank);
               }
            } else {
               COED.write(uint32_t(0));
            }
            COED.write(entry.condition.value);
            COED.close();
         }
      }
   }
   /*static*/ void container_data::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t formID;
      if (subrecord.signature() == 'CNTO') {
         if (subrecord.read(formID))
            uib.add_outbound_reference(formID);
         // remaining bytes don't matter
         return;
      }
      if (subrecord.signature() == 'COED') {
         //
         // There's a formID followed by an integer/formID union whose type depends on the form 
         // type of the formID preceding it. Fortunately, by the time we're building Use Info, 
         // we've already identified all forms and their types.
         //
         if (subrecord.read(formID)) {
            uib.add_outbound_reference(formID);
            //
            if (formID) {
               const auto* ownerStub = subrecord.lookup_form_by_id(formID);
               if (ownerStub && ownerStub->formType == form_type::actor_base) {
                  if (subrecord.read(formID)) // owner GLOB
                     uib.add_outbound_reference(formID);
               }
            }
         }
         return;
      }
      if (subrecord.signature() == 'COCT') {
         return;
      }
      #if _DEBUG
         __debugbreak();
      #else
         assert(false && "ContainerData::generateUseInfo should only be called for COCT, CNTO, and COED subrecords!");
      #endif
   }
   void container_data::clone_from(const container_data& other, form_stub& my_owner) noexcept {
      size_t size = other.entries.size();
      this->entries.clear();
      this->entries.resize(size);
      //
      for (size_t i = 0; i < size; ++i) {
         auto& entry = this->entries[i];
         auto& from  = other.entries[i];
         //
         entry.item.set(my_owner, from.item);
         entry.count = from.count;
         entry.ownership.owner.set(my_owner, from.ownership.owner);
         entry.ownership.faction_rank = from.ownership.faction_rank;
         entry.ownership.global.set(my_owner, from.ownership.global);
         entry.condition = from.condition;
      }
   }
   void container_data::sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {
      bare_form_id_t formID = target.formID;
      for (auto& entry : this->entries) {
         entry.item.clear_if(my_owner, target);
         entry.ownership.owner.clear_if(my_owner, target);
         entry.ownership.global.clear_if(my_owner, target);
      }
   }
   void container_data::clear(form_stub& my_owner) {
      for (auto& entry : this->entries) {
         entry.item.set(my_owner, nullptr);
         entry.ownership.owner.set(my_owner, nullptr);
         entry.ownership.global.set(my_owner, nullptr);
      }
      this->entries.clear();
   }
}