#include "container.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void container_data::load(tes_subrecord_reader& subrecord) {
      if (subrecord.signature() == 'CNTO') {
         auto& entry = this->entries.emplace_back();
         subrecord.unchecked_read(entry.item);
         subrecord.unchecked_read(entry.count);
         return;
      }
      if (subrecord.signature() == 'COED') {
         //
         // There's a formID followed by an integer/formID union whose type depends on the form 
         // type of the formID preceding it. Fortunately, we load all form stubs before we load 
         // any one form, so we can identify the type of the owner form from here.
         //
         if (!this->entries.size())
            return;
         auto& entry = *this->entries.rbegin();
         if (subrecord.read(entry.owner)) {
            auto ownerStub = subrecord.lookup_form_by_id(entry.owner);
            if (ownerStub && ownerStub->formType == form_type::actor_base)
               subrecord.unchecked_read(entry.global);
            else
               subrecord.unchecked_read(entry.factionRank);
            subrecord.unchecked_read(entry.condition);
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
         entry.owner.set(my_owner, from.owner);
         entry.factionRank = from.factionRank;
         entry.global.set(my_owner, from.global);
         entry.condition = from.condition;
      }
   }
   void container_data::sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {
      bare_form_id_t formID = target.formID;
      for (auto& entry : this->entries) {
         entry.item.clear_if(my_owner, target);
         entry.owner.clear_if(my_owner, target);
         entry.global.clear_if(my_owner, target);
      }
   }
}