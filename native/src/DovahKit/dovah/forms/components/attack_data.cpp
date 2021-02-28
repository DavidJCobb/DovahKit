#include "attack_data.h"
#include "../_common_cpp.h"
#include "../../notice_code_list.h"

namespace dovah::loaded_forms::components {
   void attack_data::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case 'ATKR':
         case 'ATKD':
         case 'ATKE':
            break;
         default: // invalid
            assert(false && "Why was attack_data::load called on a subrecord it's not built to handle?");
      }
      //
      if (signature == 'ATKR') {
         if (subrecord.read(this->race)) {
            intfc.log_load_warning(
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::race, intfc.target_stub, this->race)
            );
         }
         return;
      }
      if (signature == 'ATKD') {
         if (subrecord.is_in_bounds(0x2C)) {
            subrecord.unchecked_read(this->damage_mult);
            subrecord.unchecked_read(this->attack_chance);
            if (subrecord.read(this->attack_spell)) {
               intfc.log_load_warning(
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), { form_type::spell, form_type::shout }, intfc.target_stub, this->attack_spell)
               );
            }
            subrecord.unchecked_read(this->flags);
            subrecord.unchecked_read(this->attack_angle);
            subrecord.unchecked_read(this->strike_angle);
            subrecord.unchecked_read(this->stagger);
            if (subrecord.read(this->keyword)) {
               intfc.log_load_warning(
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::keyword, intfc.target_stub, this->keyword)
               );
            }
            subrecord.unchecked_read(this->knockdown);
            subrecord.unchecked_read(this->recovery_time);
            subrecord.unchecked_read(this->stamina_mult);
         }
         //
         // The game assumes that the subrecord after ATKD is ATKE. Bethesda tried to check the signature, but 
         // made a mistake: they retrieve the signature, but they never actually do check it, and the caller(s) 
         // are arranged such that they've already opened the next subrecord anyway and would therefore skip it 
         // even if they did detect a mismatch.
         //
         auto& next = record.next_subrecord();
         if (next.signature() != 'ATKE') {
            detailed_notice warning;
            warning.code = notice_code::attack_data_expected_event_subrecord;
            warning.set_cause_form(intfc.target_stub);
            warning.set_cause_subrecord(next.signature());
            intfc.log_load_warning(warning);
         }
         subrecord.to_string(this->event);
         return;
      }
      if (signature == 'ATKE') {
         subrecord.to_string(this->event);
         return;
      }
   }
   void attack_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      record.write_formID_subrecord('ATKR', this->race, true);
      //
      // TODO: under what conditions do we write ATKD+ATKE?
      //
      auto& ATKD = record.open_next_subrecord('ATKD');
      ATKD.write(this->damage_mult);
      ATKD.write(this->attack_chance);
      ATKD.write(this->attack_spell);
      ATKD.write(this->flags);
      ATKD.write(this->attack_angle);
      ATKD.write(this->strike_angle);
      ATKD.write(this->stagger);
      ATKD.write(this->keyword);
      ATKD.write(this->knockdown);
      ATKD.write(this->recovery_time);
      ATKD.write(this->stamina_mult);
      ATKD.close();
      record.write_string_subrecord('ATKE', this->event);
   }
   void attack_data::clone_from(const attack_data& original, loaded_forms::Form& owner_of_clone) noexcept {
      this->race.set(owner_of_clone, original.race);
      //
      this->damage_mult = original.damage_mult;
      this->attack_chance = original.attack_chance;
      this->attack_spell.set(owner_of_clone, original.attack_spell);
      this->flags = original.flags;
      this->attack_angle = original.attack_angle;
      this->strike_angle = original.strike_angle;
      this->stagger = original.stagger;
      this->keyword.set(owner_of_clone, original.keyword);
      this->knockdown = original.knockdown;
      this->recovery_time = original.recovery_time;
      this->stamina_mult = original.stamina_mult;
      //
      this->event = original.event;
   }
   void attack_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->race.clear_if(my_owner, target);
      this->attack_spell.clear_if(my_owner, target);
      this->keyword.clear_if(my_owner, target);
   }
   void attack_data::clear(loaded_forms::Form& my_owner) {
      this->race.set(my_owner, nullptr);
      //
      this->damage_mult = 1.0F;
      this->attack_chance = 1.0F;
      this->attack_spell.set(my_owner, nullptr);
      this->flags = 0;
      this->attack_angle = 0;
      this->strike_angle = 0;
      this->stagger = 0;
      this->keyword.set(my_owner, nullptr);
      this->knockdown = 0;
      this->recovery_time = 0;
      this->stamina_mult = 1;
      //
      this->event.clear();
   }

   void attack_data::use_info_state::read(tes_record_reader& record) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case 'ATKR':
         case 'ATKD':
         case 'ATKE':
            break;
         default: // invalid
            assert(false && "Why was attack_data::generate_use_info called on a subrecord it's not built to handle?");
      }
      //
      form_id_t form_id;
      if (signature == 'ATKR') {
         subrecord.read(this->race);
         return;
      }
      if (signature == 'ATKD') {
         if (subrecord.is_in_bounds(0x2C)) {
            subrecord.skip_bytes(8);
            subrecord.read(this->race);
            subrecord.read(this->spell);
            subrecord.skip_bytes(0x10);
            subrecord.read(this->keyword);
            subrecord.skip_bytes(0x0C);
         }
         //
         // The game assumes that the subrecord after ATKD is ATKE. Bethesda tried to check the signature, but 
         // made a mistake: they retrieve the signature, but they never actually do check it, and the caller(s) 
         // are arranged such that they've already opened the next subrecord anyway and would therefore skip it 
         // even if they did detect a mismatch.
         //
         auto& next = record.next_subrecord();
         return;
      }
   }
   void attack_data::use_info_state::commit(form_stub_use_info_builder& uib) {
      uib.add_outbound_reference(this->race);
      uib.add_outbound_reference(this->spell);
      uib.add_outbound_reference(this->keyword);
   }
}