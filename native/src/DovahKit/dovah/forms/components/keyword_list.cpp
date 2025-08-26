#include "keyword_list.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void keyword_list::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      uint32_t keywordSize = 0;
      form_reference_t formID;
      switch (subrecord.signature()) {
         case subrecord_signature_count:
            if (subrecord.read(keywordSize))
               this->forms.reserve(keywordSize);
            break;
         case subrecord_signature_array:
            if (!keywordSize)
               keywordSize = subrecord.size() / 4;
            for (uint32_t i = 0; i < keywordSize; i++) {
               if (subrecord.read(formID)) {
                  this->forms.push_back(formID);
                  intfc.warn_if_ref_is_wrong_type(formID, form_type::keyword, subrecord, { .nth_reference = i });
               }
            }
            break;
      }
   }
   /*static*/ void keyword_list::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      uint32_t  keywordSize = 0;
      form_id_t formID;
      switch (subrecord.signature()) {
         case subrecord_signature_count:
            break;
         case subrecord_signature_array:
            keywordSize = subrecord.size() / 4;
            for (uint32_t i = 0; i < keywordSize; i++)
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
            break;
      }
   }
   void keyword_list::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      uint32_t size = this->forms.size();
      if (!size) {
         //
         // The game often (always?) assumes that the subrecord after KSIZ is KWDA; so 
         // if we write KSIZ=0 without a KWDA, then things will break. The CK itself 
         // prefers not writing anything when the list is empty.
         //
         return;
      }
      auto& KSIZ = record.open_next_subrecord(subrecord_signature_count);
      KSIZ.write(size);
      KSIZ.close();
      auto& KWDA = record.open_next_subrecord(subrecord_signature_array);
      for (auto& k : this->forms)
         KWDA.write(k);
      KWDA.close();
   }
   void keyword_list::clear(loaded_forms::Form& my_owner) noexcept {
      clear_form_reference_list(this->forms, my_owner);
   }
   void keyword_list::clone_from(const keyword_list& other, loaded_forms::Form& my_owner) noexcept {
      copy_form_reference_list(my_owner, this->forms, other.forms);
   }
   void keyword_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      remove_form_from_reference_list(this->forms, target, my_owner);
   }
}