#include "Scroll.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Scroll::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            // Notably, ScrollItem actually subclasses SpellItem.
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
               // Below are the components on SpellItem.
               //
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'ETYP':
               if (auto& form = this->equip_type; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::equip_slot, subrecord.signature());
               }
               break;
            case 'MDOB':
               if (auto& form = this->menu_display_object; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::statik, subrecord.signature());
               }
               break;
               //
               // Below are the components on ScrollItem.
               //
            case 'DATA':
               subrecord.read(this->value);
               subrecord.read(this->weight);
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
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'YNAM': // BGSPickupPutdownSounds
               if (auto& form = this->sounds.take; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            case 'ZNAM': // BGSPickupPutdownSounds
               if (auto& form = this->sounds.drop; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;

               //
               // Unique subrecord below.
               //
            case components::common_spell_data::subrecord_signature:
               this->common_data.load(subrecord, intfc);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      this->effects.do_post_load_correctness_checks(intfc);
   }
   /*static*/ void Scroll::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t equip_type;
      form_id_t menu_display_object;
      components::common_spell_data::use_info_state common_spell_data;
      components::magic_effect_list::use_info_state magic_effect_list;
      struct {
         form_id_t take;
         form_id_t drop;
      } sounds;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case components::magic_effect_list::subrecord_signature_effect:
            case components::magic_effect_list::subrecord_signature_details:
               magic_effect_list.read(record);
               break;
            case components::common_spell_data::subrecord_signature:
               common_spell_data.read(subrecord);
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
            case 'DESC':
               break;

            case 'ETYP':
               subrecord.read(equip_type);
               break;
            case 'MDOB':
               subrecord.read(menu_display_object);
               break;

            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'YNAM':
               subrecord.read(sounds.take);
               break;
            case 'ZNAM':
               subrecord.read(sounds.drop);
               break;
         }
      }
      uib.add_outbound_reference(equip_type);
      uib.add_outbound_reference(menu_display_object);
      uib.add_outbound_reference(sounds.take);
      uib.add_outbound_reference(sounds.drop);
      common_spell_data.commit(uib);
      magic_effect_list.commit(uib);
      destruction_uib.done();
   }
   void Scroll::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Scroll*)out;

      copy->bounds = this->bounds;
      copy->common_data.clone_from(this->common_data, *copy);
      {
         auto& src_opt = this->destruction_data;
         auto& dst_opt = copy->destruction_data;
         if (dst_opt.has_value()) {
            dst_opt.value().clear(*copy);
            dst_opt = {};
         }
         if (src_opt.has_value()) {
            dst_opt.emplace();
            dst_opt.value().clone_from(src_opt.value(), *copy);
         }
      }
      copy->effects.clone_from(this->effects, *copy);
      copy->keywords.clone_from(this->keywords, *copy);
      copy->model.clone_from(this->model, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name        = this->name;
      copy->description = this->description;

      copy->equip_type.set(*copy, this->equip_type);
      copy->menu_display_object.set(*copy, this->menu_display_object);
      copy->sounds.take.set(*copy, this->sounds.take);
      copy->sounds.drop.set(*copy, this->sounds.drop);

      copy->value  = this->value;
      copy->weight = this->weight;
   }
   void Scroll::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
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
      record.write_formID_subrecord('MDOB', this->menu_display_object, true);
      record.write_formID_subrecord('ETYP', this->equip_type, true);
      {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('YNAM', this->sounds.take, true);
      record.write_formID_subrecord('ZNAM', this->sounds.drop, true);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->value);
         subrecord.write(this->weight);
         subrecord.close();
      }
      this->common_data.save(record, intfc);
      this->effects.save(record, intfc);
   }
   void Scroll::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->common_data.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->effects.sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->equip_type.clear_if(*this, other);
      this->menu_display_object.clear_if(*this, other);
      this->sounds.take.clear_if(*this, other);
      this->sounds.drop.clear_if(*this, other);
   }
   void Scroll::_clear_impl() noexcept {
      this->bounds.clear();
      this->common_data.clear(*this);
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

      this->equip_type.set(*this, nullptr);
      this->menu_display_object.set(*this, nullptr);
      this->sounds.take.set(*this, nullptr);
      this->sounds.drop.set(*this, nullptr);

      this->value  = 0;
      this->weight = 0;
   }
}