#include "./harvestable.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void harvestable::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_ingredient:
         case subrecord_signature_sound:
         case subrecord_signature_percentages:
            break;
         default: // invalid
            assert(false && "Why was harvestable::load called on a subrecord it's not built to handle?");
      }
      //
      if (signature == subrecord_signature_ingredient) {
         if (auto& dst = this->ingredient; subrecord.read(dst)) {
            intfc.warn_if_ref_is_wrong_type(dst, form_type::ingredient, subrecord.signature());
         }
         return;
      }
      if (signature == subrecord_signature_sound) {
         if (auto& dst = this->harvest_sound; subrecord.read(dst)) {
            intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
         }
         return;
      }
      if (signature == subrecord_signature_percentages) {
         subrecord.read(this->chance_by_season.spring);
         subrecord.read(this->chance_by_season.summer);
         subrecord.read(this->chance_by_season.autumn);
         subrecord.read(this->chance_by_season.winter);
         return;
      }
   }
   void harvestable::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      record.write_formID_subrecord(subrecord_signature_ingredient, this->ingredient,    true);
      record.write_formID_subrecord(subrecord_signature_sound,      this->harvest_sound, true);
      auto& PFPC = record.open_next_subrecord('PFPC');
      PFPC.write(this->chance_by_season.spring);
      PFPC.write(this->chance_by_season.summer);
      PFPC.write(this->chance_by_season.autumn);
      PFPC.write(this->chance_by_season.winter);
      PFPC.close();
   }
   void harvestable::clone_from(const harvestable& original, loaded_forms::Form& owner_of_clone) noexcept {
      this->ingredient.set(owner_of_clone, original.ingredient);
      this->harvest_sound.set(owner_of_clone, original.harvest_sound);
      this->chance_by_season = original.chance_by_season;
   }
   void harvestable::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->ingredient.clear_if(my_owner, target);
      this->harvest_sound.clear_if(my_owner, target);
   }
   void harvestable::clear(loaded_forms::Form& my_owner) {
      this->ingredient.set(my_owner, nullptr);
      this->harvest_sound.set(my_owner, nullptr);
      this->chance_by_season = {};
   }

   void harvestable::use_info_state::read(tes_record_reader& record) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_ingredient:
         case subrecord_signature_sound:
         case subrecord_signature_percentages:
            break;
         default: // invalid
            assert(false && "Why was harvestable::generate_use_info called on a subrecord it's not built to handle?");
      }
      //
      form_id_t form_id;
      if (signature == subrecord_signature_ingredient) {
         subrecord.read(this->ingredient);
         return;
      }
      if (signature == subrecord_signature_sound) {
         subrecord.read(this->harvest_sound);
         return;
      }
   }
   void harvestable::use_info_state::commit(form_stub_use_info_builder& uib) {
      uib.add_outbound_reference(this->ingredient);
      uib.add_outbound_reference(this->harvest_sound);
   }
}