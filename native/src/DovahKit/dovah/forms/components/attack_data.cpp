#include "attack_data.h"
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
   /*static*/ void attack_data::generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
}