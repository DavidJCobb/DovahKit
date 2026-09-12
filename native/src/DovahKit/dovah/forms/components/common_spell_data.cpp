#include "common_spell_data.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/illegal_magic_spell_type.h"

namespace {
   namespace specific_load_warnings {
      using illegal_magic_spell_type = dovah::notices::form_load_warnings::illegal_magic_spell_type;
   }
}

namespace dovah::loaded_forms::components {
   void common_spell_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      switch (subrecord.signature()) {
         case subrecord_signature:
            subrecord.read(this->base_cost);
            subrecord.read(this->flags);
            subrecord.read(this->type);
            subrecord.read(this->charge_time);
            subrecord.read(this->casting_type);
            subrecord.read(this->delivery_type);
            subrecord.read(this->casting_duration);
            subrecord.read(this->range);
            if (auto& form = this->half_cost_perk; subrecord.read(form)) {
               intfc.warn_if_ref_is_wrong_type(form, form_type::perk, subrecord);
            }

            {
               bool found = false;
               for (auto desired : legal_spell_types) {
                  if (this->type == desired) {
                     found = true;
                     break;
                  }
               }
               if (!found) {
                  specific_load_warnings::illegal_magic_spell_type notice(
                     intfc.target_stub,
                     this->type
                  );
                  intfc.log_load_warning(notice);
               }
            }

            break;
      }
   }
   void common_spell_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& subrecord = record.open_next_subrecord(subrecord_signature);
      subrecord.reserve_more(0x24);
      subrecord.write(this->base_cost);
      subrecord.write(this->flags);
      subrecord.write(this->type);
      subrecord.write(this->charge_time);
      subrecord.write(this->casting_type);
      subrecord.write(this->delivery_type);
      subrecord.write(this->casting_duration);
      subrecord.write(this->range);
      subrecord.write(this->half_cost_perk);
      subrecord.close();
   }
   void common_spell_data::clear(loaded_forms::Form& my_owner) noexcept {
      this->flags            = 0;
      this->type             = magic_spell_type::spell;
      this->base_cost        = 0;
      this->charge_time      = 0;
      this->casting_type     = magic_casting_type::constant_effect;
      this->delivery_type    = magic_delivery_type::self;
      this->casting_duration = 0;
      this->range            = 0;
      this->half_cost_perk.set(my_owner, nullptr);
   }
   void common_spell_data::clone_from(const common_spell_data& other, loaded_forms::Form& my_owner) noexcept {
      this->flags            = other.flags;
      this->type             = other.type;
      this->base_cost        = other.base_cost;
      this->charge_time      = other.charge_time;
      this->casting_type     = other.casting_type;
      this->delivery_type    = other.delivery_type;
      this->casting_duration = other.casting_duration;
      this->range            = other.range;
      this->half_cost_perk.set(my_owner, other.half_cost_perk);
   }
   void common_spell_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->half_cost_perk.clear_if(my_owner, target);
   }

   void common_spell_data::use_info_state::read(tes_subrecord_reader& subrecord) {
      subrecord.skip_bytes(
         sizeof(base_cost) +
         sizeof(flags) +
         sizeof(type) +
         sizeof(charge_time) +
         sizeof(casting_type) +
         sizeof(delivery_type) +
         sizeof(casting_duration) +
         sizeof(range)
      );
      subrecord.read(this->half_cost_perk);
   }
   void common_spell_data::use_info_state::commit(form_stub_use_info_builder& uib) {
      if (this->half_cost_perk)
         uib.add_outbound_reference(this->half_cost_perk);
   }
}