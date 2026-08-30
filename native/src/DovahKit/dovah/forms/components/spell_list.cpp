#include "spell_list.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void spell_list::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      uint32_t         count = 0;
      form_reference_t form_id;
      switch (subrecord.signature()) {
         case subrecord_signature_count:
            if (subrecord.read(count))
               this->forms.reserve(count);
            break;
         case subrecord_signature_entry:
            if (subrecord.read(form_id)) {
               this->forms.push_back(form_id);
               intfc.warn_if_ref_is_wrong_type(form_id, std::array{ form_type::spell, form_type::shout, form_type::leveled_spell }, subrecord.signature());
            }
            break;
      }
   }
   /*static*/ void spell_list::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t form_id;
      switch (subrecord.signature()) {
         case subrecord_signature_count:
            break;
         case subrecord_signature_entry:
            if (subrecord.read(form_id))
               uib.add_outbound_reference(form_id);
            break;
      }
   }
   void spell_list::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      uint32_t size = this->forms.size();
      if (!size)
         return;
      {
         auto& sub = record.open_next_subrecord(subrecord_signature_count);
         sub.write(size);
         sub.close();
      }
      for (auto& entry : this->forms)
         record.write_formID_subrecord(subrecord_signature_entry, entry, true);
   }
   void spell_list::clear(loaded_forms::Form& my_owner) noexcept {
      clear_form_reference_list(this->forms, my_owner);
   }
   void spell_list::clone_from(const spell_list& other, loaded_forms::Form& my_owner) noexcept {
      copy_form_reference_list(my_owner, this->forms, other.forms);
   }
   void spell_list::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      remove_form_from_reference_list(this->forms, target, my_owner);
   }
}