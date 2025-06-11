#include "CombatStyle.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void CombatStyle::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;

            case 'CSGD':
               subrecord.read(this->general.offensive_mult);
               subrecord.read(this->general.defensive_mult);
               subrecord.read(this->general.group_offensive_mult);
               subrecord.read(this->general.equipment_score_mults.melee);
               subrecord.read(this->general.equipment_score_mults.magic);
               subrecord.read(this->general.equipment_score_mults.ranged);
               subrecord.read(this->general.equipment_score_mults.shout);
               subrecord.read(this->general.equipment_score_mults.unarmed);
               subrecord.read(this->general.equipment_score_mults.staff);
               subrecord.read(this->general.avoid_threat_chance);
               break;
            case 'CSME':
               subrecord.read(this->melee.attack_staggered_mult);
               subrecord.read(this->melee.power_attack_staggered_mult);
               subrecord.read(this->melee.power_attack_blocking_mult);
               subrecord.read(this->melee.bash_mult);
               subrecord.read(this->melee.bash_recoil_mult);
               subrecord.read(this->melee.bash_attack_mult);
               subrecord.read(this->melee.bash_power_attack_mult);
               subrecord.read(this->melee.special_attack_mult);
               break;
            case 'CSCR':
               subrecord.read(this->close_range.circle_mult);
               subrecord.read(this->close_range.fallback_mult);
               subrecord.read(this->close_range.flank_distance);
               subrecord.read(this->close_range.stalk_time);
               break;
            case 'CSLR':
               subrecord.read(this->long_range.strafe_mult);
               break;
            case 'CSFL':
               subrecord.read(this->flight.hover.chance);
               subrecord.read(this->flight.divebomb.chance);
               subrecord.read(this->flight.ground_attack.chance);
               subrecord.read(this->flight.hover.time);
               subrecord.read(this->flight.ground_attack.time);
               subrecord.read(this->flight.perch_attack.chance);
               subrecord.read(this->flight.perch_attack.time);
               subrecord.read(this->flight.flying_attack.chance);
               break;
            case 'DATA':
               subrecord.read(this->flags);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void CombatStyle::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void CombatStyle::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (CombatStyle*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->flags = this->flags;
      copy->general     = this->general;
      copy->melee       = this->melee;
      copy->close_range = this->close_range;
      copy->long_range  = this->long_range;
      copy->flight      = this->flight;
   }
   void CombatStyle::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('CSGD');
         subrecord.write(this->general.offensive_mult);
         subrecord.write(this->general.defensive_mult);
         subrecord.write(this->general.group_offensive_mult);
         subrecord.write(this->general.equipment_score_mults.melee);
         subrecord.write(this->general.equipment_score_mults.magic);
         subrecord.write(this->general.equipment_score_mults.ranged);
         subrecord.write(this->general.equipment_score_mults.shout);
         subrecord.write(this->general.equipment_score_mults.unarmed);
         subrecord.write(this->general.equipment_score_mults.staff);
         subrecord.write(this->general.avoid_threat_chance);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('CSME');
         subrecord.write(this->melee.attack_staggered_mult);
         subrecord.write(this->melee.power_attack_staggered_mult);
         subrecord.write(this->melee.power_attack_blocking_mult);
         subrecord.write(this->melee.bash_mult);
         subrecord.write(this->melee.bash_recoil_mult);
         subrecord.write(this->melee.bash_attack_mult);
         subrecord.write(this->melee.bash_power_attack_mult);
         subrecord.write(this->melee.special_attack_mult);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('CSCR');
         subrecord.write(this->close_range.circle_mult);
         subrecord.write(this->close_range.fallback_mult);
         subrecord.write(this->close_range.flank_distance);
         subrecord.write(this->close_range.stalk_time);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('CSLR');
         subrecord.write(this->long_range.strafe_mult);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('CSFL');
         subrecord.write(this->flight.hover.chance);
         subrecord.write(this->flight.divebomb.chance);
         subrecord.write(this->flight.ground_attack.chance);
         subrecord.write(this->flight.hover.time);
         subrecord.write(this->flight.ground_attack.time);
         subrecord.write(this->flight.perch_attack.chance);
         subrecord.write(this->flight.perch_attack.time);
         subrecord.write(this->flight.flying_attack.chance);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->flags);
         subrecord.close();
      }
   }
   void CombatStyle::_clear_impl() noexcept {
      this->script_data.clear(*this);
      
      this->flags = 0;
      this->general     = {};
      this->melee       = {};
      this->close_range = {};
      this->long_range  = {};
      this->flight      = {};
   }
   void CombatStyle::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}