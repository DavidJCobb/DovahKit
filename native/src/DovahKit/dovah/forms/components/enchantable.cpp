#include "./enchantable.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void enchantable::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      auto signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_effect:
         case subrecord_signature_effect_legacy:
         case subrecord_signature_charge:
         case subrecord_signature_charge_legacy:
            break;
         default: // invalid
            assert(false && "Why was harvestable::load called on a subrecord it's not built to handle?");
      }
      
      if (signature == subrecord_signature_effect || signature == subrecord_signature_effect_legacy) {
         if (auto& dst = this->effect; subrecord.read(dst)) {
            intfc.warn_if_ref_is_wrong_type(dst, std::array{ form_type::enchantment, form_type::spell }, subrecord.signature());
         }
         return;
      }
      if (signature == subrecord_signature_charge || signature == subrecord_signature_charge_legacy) {
         subrecord.read(this->charge);
         return;
      }
   }
   void enchantable::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      record.write_formID_subrecord(subrecord_signature_effect, this->effect, true);
      if (this->charge) {
         auto& subrecord = record.open_next_subrecord(subrecord_signature_charge);
         subrecord.write(this->charge);
         subrecord.close();
      }
   }
   void enchantable::clone_from(const enchantable& original, loaded_forms::Form& owner_of_clone) noexcept {
      this->effect.set(owner_of_clone, original.effect);
      this->charge = original.charge;
   }
   void enchantable::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->effect.clear_if(my_owner, target);
   }
   void enchantable::clear(loaded_forms::Form& my_owner) {
      this->effect.set(my_owner, nullptr);
      this->charge = {};
   }

   void enchantable::use_info_state::read(tes_subrecord_reader& subrecord) {
      auto signature = subrecord.signature();
      if (signature == subrecord_signature_effect || signature == subrecord_signature_effect_legacy) {
         subrecord.read(this->effect);
         return;
      }
   }
   void enchantable::use_info_state::commit(form_stub_use_info_builder& uib) {
      uib.add_outbound_reference(this->effect);
   }
}