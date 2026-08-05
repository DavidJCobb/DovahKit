#include "Enchantment.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/illegal_magic_spell_type.h"

namespace {
   namespace specific_load_warnings {
      using illegal_magic_spell_type = dovah::notices::form_load_warnings::illegal_magic_spell_type;
   }
}

namespace dovah::loaded_forms {
   void Enchantment::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            //
            // The general pattern that the game and Creation Kit use for MagicItem forms 
            // (ALCH, ENCH, INGR, SPEL) is as follows: none of them override the virtual 
            // "Load" function on MagicItem. Instead, they all define a handful of other 
            // virtual functions which the MagicItem loader calls:
            // 
            //  - A virtual function to load any form components that are unique to the 
            //    subclass, e.g. BGSMenuDisplayObject on SpellItem.
            // 
            //  - Virtual functions which allow a subclass to define a single subrecord 
            //    signature, and to load that one subrecord.
            // 
            // Below are the subrecords common to MagicItem.
            //
            case 'EDID': // already read by the FormStub
               break;
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case components::magic_effect_list::subrecord_signature_effect:
            case components::magic_effect_list::subrecord_signature_details:
            case 'CTDA':
               this->effects.load(record, intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            //
            // EnchantmentItem has no form components of its own, so there's just the 
            // single unique subrecord.
            //
            ;
            //
            // Unique subrecord below.
            //
            case 'ENIT':
               subrecord.read(this->cost);
               subrecord.read(this->flags);
               subrecord.read(this->casting_type);
               subrecord.read(this->charge_amount);
               subrecord.read(this->delivery_type);
               subrecord.read(this->enchantment_type);
               subrecord.read(this->charge_time);
               if (auto& form = this->base_enchantment; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::enchantment, subrecord.signature());
               if (auto& form = this->worn_restrictions; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::formlist, subrecord.signature());
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }

      {
         bool found = false;
         for (auto desired : legal_spell_types) {
            if (this->enchantment_type == desired) {
               found = true;
               break;
            }
         }
         if (!found) {
            specific_load_warnings::illegal_magic_spell_type notice(
               this->stub,
               this->enchantment_type
            );
            intfc.log_load_warning(notice);
         }
      }

      this->effects.do_post_load_correctness_checks(intfc);
   }
   /*static*/ void Enchantment::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t base_enchantment;
      form_id_t worn_restrictions;
      components::magic_effect_list::use_info_state magic_effect_list;

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case components::magic_effect_list::subrecord_signature_effect:
            case components::magic_effect_list::subrecord_signature_details:
               magic_effect_list.read(record);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;

            case 'FULL':
               break;

            case 'ENIT':
               subrecord.skip_bytes(
                  sizeof(cost) +
                  sizeof(flags) +
                  sizeof(casting_type) +
                  sizeof(charge_amount) +
                  sizeof(delivery_type) +
                  sizeof(enchantment_type) +
                  sizeof(charge_time)
               );
               subrecord.read(base_enchantment);
               subrecord.read(worn_restrictions);
         }
      }
      uib.add_outbound_reference(base_enchantment);
      uib.add_outbound_reference(worn_restrictions);
      magic_effect_list.commit(uib);
   }
   void Enchantment::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Enchantment*)out;

      copy->bounds = this->bounds;
      copy->effects.clone_from(this->effects, *copy);
      copy->keywords.clone_from(this->keywords, *copy);
      copy->script_data.clone_from(this->script_data, *copy);
      copy->name = this->name;

      copy->cost = this->cost;
      copy->flags = this->flags;
      copy->casting_type = this->casting_type;
      copy->charge_amount = this->charge_amount;
      copy->delivery_type = this->delivery_type;
      copy->enchantment_type = this->enchantment_type;
      copy->charge_time = this->charge_time;
      copy->base_enchantment.set(*copy, this->base_enchantment);
      copy->worn_restrictions.set(*copy, this->worn_restrictions);
   }
   void Enchantment::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      if (!this->name.empty()) {
         auto& subrecord = record.open_next_subrecord('FULL');
         subrecord.write(this->name);
         subrecord.close();
      }
      this->keywords.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('ENIT');
         subrecord.write(this->cost);
         subrecord.write(this->flags);
         subrecord.write(this->casting_type);
         subrecord.write(this->charge_amount);
         subrecord.write(this->delivery_type);
         subrecord.write(this->enchantment_type);
         subrecord.write(this->charge_time);
         subrecord.write(this->base_enchantment);
         subrecord.write(this->worn_restrictions);
         subrecord.close();
      }
      this->effects.save(record, intfc);
   }
   void Enchantment::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->effects.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->base_enchantment.clear_if(*this, other);
      this->worn_restrictions.clear_if(*this, other);
   }
   void Enchantment::_clear_impl() noexcept {
      this->bounds.clear();
      this->effects.clear(*this);
      this->keywords.clear(*this);
      this->script_data.clear(*this);

      this->name.reset();

      this->cost = 0;
      this->flags = 0;
      this->charge_amount = 0;
      this->charge_time = 0;
      this->casting_type = {};
      this->delivery_type = {};
      this->enchantment_type = magic_spell_type::enchantment_normal;

      this->base_enchantment.set(*this, nullptr);
      this->worn_restrictions.set(*this, nullptr);
   }
}