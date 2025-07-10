#include "Apparatus.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Apparatus::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case components::object_bounds::subrecord:
               this->bounds.load(subrecord, intfc);
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
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'ICON':
               subrecord.read(this->icons.inventory);
               break;
            case 'MICO':
               subrecord.read(this->icons.message);
               break;
            case 'DATA':
               subrecord.read(this->value);
               subrecord.read(this->weight);
               break;
            case 'QUAL':
               subrecord.read(this->quality);
               break;
            case 'YNAM':
               if (auto& form = this->sounds.take; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            case 'ZNAM':
               if (auto& form = this->sounds.drop; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Apparatus::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      components::destruction_stage_data::use_info_builder destruction_uib(uib);
      struct {
         form_id_t take;
         form_id_t drop;
      } sounds;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               decltype(model)::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case components::object_bounds::subrecord:
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'YNAM':
               subrecord.read(sounds.take);
               break;
            case 'ZNAM':
               subrecord.read(sounds.drop);
               break;
         }
      }
      uib.add_outbound_reference(sounds.take);
      uib.add_outbound_reference(sounds.drop);
      destruction_uib.done();
   }
   void Apparatus::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Apparatus*)out;
      
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

      copy->name = this->name;
      copy->description = this->description;

      copy->icons = this->icons;
      copy->quality = this->quality;
      copy->value = this->value;
      copy->weight = this->weight;

      copy->sounds.take.set(*copy, this->sounds.take);
      copy->sounds.drop.set(*copy, this->sounds.drop);
   }
   void Apparatus::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (!this->icons.inventory.empty())
         record.write_string_subrecord('ICON', this->icons.inventory);
      if (!this->icons.message.empty())
         record.write_string_subrecord('MICO', this->icons.message);
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      record.write_formID_subrecord('YNAM', this->sounds.take, true);
      record.write_formID_subrecord('ZNAM', this->sounds.drop, true);
      {
         auto& subrecord = record.open_next_subrecord('QUAL');
         subrecord.write(this->quality);
         subrecord.close();
      }
      auto& DESC = record.open_next_subrecord('DESC');
      DESC.write(this->description);
      DESC.close();
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->value);
      DATA.write(this->weight);
      DATA.close();
   }
   void Apparatus::_clear_impl() noexcept {
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->model.clear(*this);
      this->script_data.clear(*this);
      this->sounds.take.set(*this, nullptr);
      this->sounds.drop.set(*this, nullptr);
      this->value  = 0;
      this->weight = 0;
      this->name.reset();
      this->description.reset();
      this->icons = {};
      this->quality = skill_level::novice;
   }
   void Apparatus::_sever_outbound_references_impl(form_stub& other) noexcept {
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->sounds.take.clear_if(*this, other);
      this->sounds.drop.clear_if(*this, other);
   }
}