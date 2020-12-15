#include "keyword_list.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void keyword_list::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      uint32_t keywordSize = 0;
      form_reference_t formID;
      switch (subrecord.signature()) {
         case 'KSIZ':
            if (subrecord.read(keywordSize))
               this->forms.reserve(keywordSize);
            break;
         case 'KWDA':
            if (!keywordSize)
               keywordSize = subrecord.size() / 4;
            for (uint32_t i = 0; i < keywordSize; i++) {
               if (subrecord.read(formID)) {
                  this->forms.push_back(formID);
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::keyword, intfc.target_stub, formID)
                        .set_cause_form_index(i)
                  );
               }
            }
            break;
      }
   }
   /*static*/ void keyword_list::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      uint32_t  keywordSize = 0;
      form_id_t formID;
      switch (subrecord.signature()) {
         case 'KSIZ':
            break;
         case 'KWDA':
            keywordSize = subrecord.size() / 4;
            for (uint32_t i = 0; i < keywordSize; i++)
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
            break;
      }
   }
   void keyword_list::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      uint32_t size = this->forms.size();
      if (!size)
         return;
      auto& KSIZ = record.open_next_subrecord('KSIZ');
      KSIZ.write(size);
      KSIZ.close();
      auto& KWDA = record.open_next_subrecord('KWDA');
      for (auto& k : this->forms)
         KWDA.write(k);
      KWDA.close();
   }
   void keyword_list::clear(form_stub& my_owner) noexcept {
      for (auto& id : this->forms)
         id.set(my_owner, nullptr);
      this->forms.clear();
   }
   void keyword_list::clone_from(const keyword_list& other, form_stub& my_owner) noexcept {
      size_t size = other.forms.size();
      this->clear(my_owner);
      this->forms.resize(size);
      for (size_t i = 0; i < size; ++i)
         this->forms[i].set(my_owner, other.forms[i]);
   }
   void keyword_list::sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {
      auto& list  = this->forms;
      bool  edits = false;
      for (auto& id : list) {
         if (id == &target) {
            id.set(my_owner, nullptr);
            edits = true;
         }
      }
      if (edits) {
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [](form_reference_t& id) {
                  return id == nullptr;
               }
            ),
            list.end()
         );
      }
   }
}