#include "SoulGem.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void SoulGem::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      bool content_loaded = false;
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case components::object_bounds::subrecord:
               this->bounds.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'ICON':
               subrecord.read(this->icon);
               break;
            case 'MICO':
               subrecord.read(this->message_icon);
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case components::keyword_list::subrecord_signature_count:
            case components::keyword_list::subrecord_signature_array:
               this->keywords.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->value);
               subrecord.read(this->weight);
               break;
            case 'YNAM':
               if (subrecord.read(this->take_sound)) {
                  intfc.warn_if_ref_is_wrong_type(this->take_sound, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            case 'ZNAM':
               if (subrecord.read(this->drop_sound)) {
                  intfc.warn_if_ref_is_wrong_type(this->drop_sound, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            //
            // SoulGem fields:
            //
            case 'SOUL':
               subrecord.read(this->initial_soul_size);
               break;
            case 'SLCP':
               subrecord.read(this->maximum_soul_size);
               break;
            case 'NAM0':
               if (auto& dst = this->linked_to; subrecord.read(dst)) {
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::soul_gem, subrecord.signature());
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void SoulGem::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      form_id_t take_sound;
      form_id_t drop_sound;
      form_id_t form_id;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);
      form_id_t linked_to; // SLGM/NAM0

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'ICON':
            case 'MICO':
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'KSIZ':
            case 'KWDA':
               components::keyword_list::generate_use_info(subrecord, uib);
               break;
            case components::object_bounds::subrecord:
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               break;
            case 'YNAM':
               subrecord.read(take_sound);
               break;
            case 'ZNAM':
               subrecord.read(drop_sound);
               break;
            case 'NAM0':
               subrecord.read(linked_to);
               break;
         }
      }
      uib.add_outbound_reference(take_sound);
      uib.add_outbound_reference(drop_sound);
      uib.add_outbound_reference(form_id);
      destruction_uib.done();
      uib.add_outbound_reference(linked_to);
   }
   void SoulGem::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (SoulGem*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
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
      copy->keywords.clone_from(this->keywords, *copy);
      copy->name         = this->name;
      copy->icon         = this->icon;
      copy->message_icon = this->message_icon;
      copy->drop_sound.set(*copy, this->drop_sound);
      copy->take_sound.set(*copy, this->take_sound);

      copy->initial_soul_size = this->initial_soul_size;
      copy->maximum_soul_size = this->maximum_soul_size;
      copy->linked_to.set(*copy, this->linked_to);
   }
   void SoulGem::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (!this->icon.empty())
         record.write_string_subrecord('ICON', this->icon);
      if (!this->message_icon.empty())
         record.write_string_subrecord('MICO', this->message_icon);
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('YNAM', this->take_sound, true);
      record.write_formID_subrecord('ZNAM', this->drop_sound, true);
      this->keywords.save(record, intfc);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->value);
      DATA.write(this->weight);
      DATA.close();
      auto& SOUL = record.open_next_subrecord('SOUL');
      SOUL.write(this->initial_soul_size);
      SOUL.close();
      auto& SLCP = record.open_next_subrecord('SLCP');
      SLCP.write(this->maximum_soul_size);
      SLCP.close();
      record.write_formID_subrecord('NAM0', this->linked_to, true);
   }
   void SoulGem::_clear_impl() noexcept {
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->keywords.clear(*this);
      this->model.clear(*this);
      this->script_data.clear(*this);
      this->drop_sound.set(*this, nullptr);
      this->take_sound.set(*this, nullptr);
      this->value  = 0;
      this->weight = 0;
      this->name.reset();
      this->icon.clear();
      this->message_icon.clear();
      
      this->initial_soul_size = soul_size::none;
      this->maximum_soul_size = soul_size::none;
      this->linked_to.set(*this, nullptr);
   }
   void SoulGem::_sever_outbound_references_impl(form_stub& other) noexcept {
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->keywords.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->drop_sound.clear_if(*this, other);
      this->take_sound.clear_if(*this, other);

      this->linked_to.clear_if(*this, other);
   }
}