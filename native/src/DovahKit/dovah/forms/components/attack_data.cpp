#include "attack_data.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_component/attack_data/expected_event_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::attack_data;
   }
}

namespace dovah::loaded_forms::components {
   void attack_data::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_race:
         case subrecord_signature_data:
         case subrecord_signature_event:
            break;
         default: // invalid
            assert(false && "Why was attack_data::load called on a subrecord it's not built to handle?");
      }
      //
      if (signature == subrecord_signature_race) {
         if (subrecord.read(this->race)) {
            intfc.warn_if_ref_is_wrong_type(this->race, form_type::race, subrecord.signature());
         }
         return;
      }
      if (signature == subrecord_signature_data) {
         auto& dst = this->attacks.emplace_back();
         if (subrecord.is_in_bounds(0x2C)) {
            subrecord.unchecked_read(dst.damage_mult);
            subrecord.unchecked_read(dst.attack_chance);
            if (subrecord.read(dst.attack_spell)) {
               intfc.warn_if_ref_is_wrong_type(dst.attack_spell, std::array{ form_type::spell, form_type::shout }, subrecord.signature());
            }
            subrecord.unchecked_read(dst.flags);
            subrecord.unchecked_read(dst.attack_angle);
            subrecord.unchecked_read(dst.strike_angle);
            subrecord.unchecked_read(dst.stagger);
            if (subrecord.read(dst.keyword)) {
               intfc.warn_if_ref_is_wrong_type(dst.keyword, form_type::keyword, subrecord.signature());
            }
            subrecord.unchecked_read(dst.knockdown);
            subrecord.unchecked_read(dst.recovery_time);
            subrecord.unchecked_read(dst.stamina_mult);
         }
         //
         // The game assumes that the subrecord after ATKD is ATKE. Bethesda tried to check the signature, but 
         // made a mistake: they retrieve the signature, but they never actually do check it, and the caller(s) 
         // are arranged such that they've already opened the next subrecord anyway and would therefore skip it 
         // even if they did detect a mismatch.
         //
         auto& next = record.next_subrecord();
         if (next.signature() != subrecord_signature_event) {
            specific_load_warnings::expected_event_subrecord notice(
               const_cast<form_stub&>(intfc.target_stub),
               next.signature()
            );
            intfc.log_load_warning(notice);
         }
         subrecord.read(dst.event);
         return;
      }
      if (signature == subrecord_signature_event) {
         if (this->attacks.empty()) {
            this->attacks.emplace_back();
         }
         auto& prev = this->attacks.back();
         subrecord.read(prev.event);
         return;
      }
   }
   void attack_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      record.write_formID_subrecord(subrecord_signature_race, this->race, true);
      //
      // TODO: under what conditions do we write ATKD+ATKE?
      //
      for (const auto& entry : this->attacks) {
         auto& ATKD = record.open_next_subrecord(subrecord_signature_data);
         ATKD.write(entry.damage_mult);
         ATKD.write(entry.attack_chance);
         ATKD.write(entry.attack_spell);
         ATKD.write(entry.flags);
         ATKD.write(entry.attack_angle);
         ATKD.write(entry.strike_angle);
         ATKD.write(entry.stagger);
         ATKD.write(entry.keyword);
         ATKD.write(entry.knockdown);
         ATKD.write(entry.recovery_time);
         ATKD.write(entry.stamina_mult);
         ATKD.close();
         record.write_string_subrecord(subrecord_signature_event, entry.event);
      }
   }
   void attack_data::clone_from(const attack_data& original, loaded_forms::Form& owner_of_clone) noexcept {
      this->race.set(owner_of_clone, original.race);
      //
      for (auto& entry : this->attacks) {
         entry.attack_spell.set(owner_of_clone, nullptr);
         entry.keyword.set(owner_of_clone, nullptr);
      }
      this->attacks.clear();

      size_t size = original.attacks.size();
      this->attacks.resize(size);
      for (size_t i = 0; i < size; ++i) {
         const auto& src = original.attacks[i];
         auto& dst = this->attacks[i];

         dst.damage_mult = src.damage_mult;
         dst.attack_chance = src.attack_chance;
         dst.attack_spell.set(owner_of_clone, src.attack_spell);
         dst.flags = src.flags;
         dst.attack_angle = src.attack_angle;
         dst.strike_angle = src.strike_angle;
         dst.stagger = src.stagger;
         dst.keyword.set(owner_of_clone, src.keyword);
         dst.knockdown = src.knockdown;
         dst.recovery_time = src.recovery_time;
         dst.stamina_mult = src.stamina_mult;
         //
         dst.event = src.event;
      }
   }
   void attack_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->race.clear_if(my_owner, target);
      for (auto& entry : this->attacks) {
         entry.attack_spell.clear_if(my_owner, target);
         entry.keyword.clear_if(my_owner, target);
      }
   }
   void attack_data::clear(loaded_forms::Form& my_owner) {
      this->race.set(my_owner, nullptr);
      //
      for (auto& entry : this->attacks) {
         entry.attack_spell.set(my_owner, nullptr);
         entry.keyword.set(my_owner, nullptr);
      }
      this->attacks.clear();
   }

   void attack_data::use_info_state::read(tes_record_reader& record) {
      auto& subrecord = record.get_current_subrecord();
      auto  signature = subrecord.signature();
      switch (signature) {
         case subrecord_signature_race:
         case subrecord_signature_data:
         case subrecord_signature_event:
            break;
         default: // invalid
            assert(false && "Why was attack_data::generate_use_info called on a subrecord it's not built to handle?");
      }
      //
      form_id_t form_id;
      if (signature == subrecord_signature_race) {
         subrecord.read(this->race);
         return;
      }
      if (signature == subrecord_signature_data) {
         if (subrecord.is_in_bounds(0x2C)) {
            subrecord.skip_bytes(8);
            subrecord.read(form_id);
            if (form_id)
               this->attack_forms.push_back(form_id);
            subrecord.skip_bytes(0x10);
            subrecord.read(form_id);
            if (form_id)
               this->attack_forms.push_back(form_id);
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
      for(auto id : this->attack_forms)
         uib.add_outbound_reference(id);
   }
}