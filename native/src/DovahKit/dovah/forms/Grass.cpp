#include "Grass.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Grass::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->density);
               subrecord.read(this->slope.min);
               subrecord.read(this->slope.max);
               subrecord.skip_bytes(1);
               subrecord.read(this->distance_from_water.units);
               subrecord.skip_bytes(2);
               subrecord.read(this->distance_from_water.comparator);
               subrecord.read(this->position_range);
               subrecord.read(this->height_range);
               subrecord.read(this->color_range);
               subrecord.read(this->wave_period);
               subrecord.read(this->grass_flags);
               subrecord.skip_bytes(3);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Grass::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
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
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void Grass::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Grass*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);

      copy->density = this->density;
      copy->slope = this->slope;
      copy->distance_from_water = this->distance_from_water;
      copy->position_range = this->position_range;
      copy->height_range = this->height_range;
      copy->color_range = this->color_range;
      copy->wave_period = this->wave_period;
      copy->grass_flags = this->grass_flags;
   }
   void Grass::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->density);
      DATA.write(this->slope.min);
      DATA.write(this->slope.max);
      DATA.skip_bytes(1);
      DATA.write(this->distance_from_water.units);
      DATA.skip_bytes(2);
      DATA.write(this->distance_from_water.comparator);
      DATA.write(this->position_range);
      DATA.write(this->height_range);
      DATA.write(this->color_range);
      DATA.write(this->wave_period);
      DATA.write(this->grass_flags);
      DATA.skip_bytes(3);
      DATA.close();
   }
   void Grass::_clear_impl() noexcept {
      this->bounds.clear();
      this->model.clear(*this);
      this->script_data.clear(*this);

      this->density = 0;
      this->slope = {};
      this->distance_from_water = {};
      this->position_range = 0;
      this->height_range = 0;
      this->color_range = 0;
      this->wave_period = 0;
      this->grass_flags = 0;
   }
   void Grass::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
   }
}