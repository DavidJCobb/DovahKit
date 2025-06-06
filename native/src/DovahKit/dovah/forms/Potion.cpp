#include "Potion.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Potion::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            // Below are the components on AlchemyItem.
            //
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'DEST': // destruction stage header // details: https://en.uesp.net/wiki/Tes5Mod:Mod_File_Format/DEST_Field
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case 'ETYP':
               if (auto& form = this->equip_type; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::equip_slot, subrecord.signature());
               break;
            case 'ICON':
               subrecord.read(this->icons.inventory); // TESIcon
               break;
            case 'MICO':
               subrecord.read(this->icons.message); // BGSMessageIcon
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->weight);
               break;
            //
            // Unique subrecord below.
            //
            case 'ENIT':
               subrecord.read(this->value);
               subrecord.read(this->flags);
               if (auto& form = this->addiction.form; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::spell, subrecord.signature());
               subrecord.read(this->addiction.chance);
               if (auto& form = this->sounds.use; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               break;
            //
            // Didn't see this in the loader, but xEdit says it's there:
            //
            case 'YNAM':
               if (auto& form = this->sounds.take; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'ZNAM':
               if (auto& form = this->sounds.drop; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      this->effects.do_post_load_correctness_checks(intfc);
   }
   /*static*/ void Potion::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      struct {
         form_id_t form;
      } addiction;
      form_id_t equip_type;
      struct {
         form_id_t take;
         form_id_t drop;
         form_id_t use;
      } sounds;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);
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

            case 'DATA':
            case 'DESC':
            case 'ICON':
            case 'MICO':
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'ETYP':
               subrecord.read(equip_type);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;

            case 'ENIT':
               subrecord.skip_bytes(
                  sizeof(value) +
                  sizeof(flags)
               );
               subrecord.read(addiction.form);
               subrecord.skip_bytes(4);
               subrecord.read(sounds.use);
               break;

            case 'YNAM':
               subrecord.read(sounds.take);
               break;
            case 'ZNAM':
               subrecord.read(sounds.drop);
               break;
         }
      }
      uib.add_outbound_reference(addiction.form);
      uib.add_outbound_reference(equip_type);
      uib.add_outbound_reference(sounds.use);
      uib.add_outbound_reference(sounds.take);
      uib.add_outbound_reference(sounds.drop);
      magic_effect_list.commit(uib);
      destruction_uib.done();
   }
   void Potion::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Potion*)out;

      copy->bounds = this->bounds;
      copy->effects.clone_from(this->effects, *copy);
      copy->keywords.clone_from(this->keywords, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name = this->name;
      copy->description = this->description;

      copy->flags  = this->flags;
      copy->value  = this->value;
      copy->weight = this->weight;
      copy->addiction.form.set(*copy, this->addiction.form);
      copy->addiction.chance = this->addiction.chance;
      copy->equip_type.set(*copy, this->equip_type);
      copy->icons = this->icons;
      copy->sounds.use.set(*copy, this->sounds.use);
      copy->sounds.take.set(*copy, this->sounds.take);
      copy->sounds.drop.set(*copy, this->sounds.drop);
   }
   void Potion::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
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
      if (!this->description.empty()) {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      if (!this->icons.inventory.empty())
         record.write_string_subrecord('ICON', this->icons.inventory);
      if (!this->icons.message.empty())
         record.write_string_subrecord('MICO', this->icons.message);
      record.write_formID_subrecord('YNAM', this->sounds.take, true);
      record.write_formID_subrecord('ZNAM', this->sounds.drop, true);
      record.write_formID_subrecord('DATA', this->equip_type, true);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->weight);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('ENIT');
         subrecord.write(this->value);
         subrecord.write(this->flags);
         subrecord.write(this->addiction.form);
         subrecord.write(this->addiction.chance);
         subrecord.write(this->sounds.use);
         subrecord.close();
      }
      this->effects.save(record, intfc);
   }
   void Potion::_sever_outbound_references_impl(form_stub& other) noexcept {
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->effects.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->addiction.form.clear_if(*this, other);
      this->equip_type.clear_if(*this, other);
      this->sounds.use.clear_if(*this, other);
      this->sounds.take.clear_if(*this, other);
      this->sounds.drop.clear_if(*this, other);
   }
   void Potion::_clear_impl() noexcept {
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->effects.clear(*this);
      this->keywords.clear(*this);
      this->model.clear(*this);
      this->script_data.clear(*this);

      this->name.reset();
      this->description.reset();

      this->flags  = 0;
      this->value  = 0;
      this->weight = 0;
      this->icons  = {};

      this->addiction.form.set(*this, nullptr);
      this->addiction.chance = 0;
      this->equip_type.set(*this, nullptr);
      this->sounds.use.set(*this, nullptr);
      this->sounds.take.set(*this, nullptr);
      this->sounds.drop.set(*this, nullptr);
   }
}