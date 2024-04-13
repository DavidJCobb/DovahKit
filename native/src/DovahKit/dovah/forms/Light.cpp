#include "Light.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void Light::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'FULL':
               subrecord.read(this->item_data.name);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS': // TODO: is LIGH swappable?
               this->model.load(subrecord, intfc);
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
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'DATA':
               {
                  subrecord.read(this->time);
                  subrecord.read(this->radius);
                  this->color.load(subrecord);
                  subrecord.read(this->light_flags);
                  {
                     this->light_type = engine_light_type::omni;
                     //
                     auto f = this->light_flags;
                     if ((f & light_flag::type_spot) != 0)
                        this->light_type = engine_light_type::spot;
                     if ((f & light_flag::type_spot_shadow) != 0)
                        this->light_type = engine_light_type::spot_shadow;
                     if ((f & light_flag::type_hemi_shadow) != 0)
                        this->light_type = engine_light_type::hemi_shadow;
                     if ((f & light_flag::type_omni_shadow) != 0)
                        this->light_type = engine_light_type::omni_shadow;
                     //
                     this->light_flags &= ~light_flag::all_types;
                  }
                  subrecord.read(this->falloff_exponent);
                  subrecord.read(this->fov);
                  subrecord.read(this->near_clip);
                  subrecord.read(this->flicker.period);
                  subrecord.read(this->flicker.amplitudes.intensity);
                  subrecord.read(this->flicker.amplitudes.movement);
                  subrecord.read(this->item_data.value);
                  subrecord.read(this->item_data.weight);
               }
               break;
            case 'FNAM':
               subrecord.read(this->fade);
               break;
            case 'SNAM':
               if (subrecord.read(this->item_data.sound)) {
                  intfc.log_load_warning(
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, this->stub, this->item_data.sound)
                  );
               }
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void Light::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      form_id_t sound;
      components::destruction_stage_data::use_info_builder destruction_uib(uib);

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL': // item name
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generate_use_info(subrecord, uib); // redundant TESModel subrecords just append more texture replacement entries, without clearing those already in the list
               break;
            case 'DEST': // destruction stage header
            case 'DSTD': // destruction stage data
            case 'DMDL': // destruction stage model
            case 'DMDT': // destruction stage model texture hashes
            case 'DMDS': // destruction stage model texture swaps
            case 'DSTF': // destruction stage end marker
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case 'OBND':
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM':
               subrecord.read(sound);
               break;
            case 'FNAM': // fade
            case 'DATA': // light and item values
               break;
         }
      }
      uib.add_outbound_reference(sound);
      destruction_uib.done();
   }
   bool Light::_clone_impl(Form* out) const noexcept {
      if (out->type != form_type)
         return false;
      auto copy = (Light*)out;
      //
      copy->item_data.sound.set(*copy, this->item_data.sound);
      copy->item_data = this->item_data;
      //
      copy->color       = this->color;
      copy->fade        = this->fade;
      copy->falloff_exponent = this->falloff_exponent;
      copy->flicker     = this->flicker;
      copy->fov         = this->fov;
      copy->light_flags = this->light_flags;
      copy->light_type  = this->light_type;
      copy->near_clip   = this->near_clip;
      copy->radius      = this->radius;
      copy->time        = this->time;
      //
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
      //
      return true;
   }
   bool Light::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->item_data.name);
      FULL.close();
      record.write_string_subrecord('ICON', this->item_data.icon);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->time);
      DATA.write(this->radius);
      this->color.save(DATA);
      {
         auto f = this->light_flags & ~light_flag::all_types;
         switch (this->light_type) {
            using enum engine_light_type;
            case omni:
               break;
            case omni_shadow:
               f |= light_flag::type_omni_shadow;
               break;
            case hemi_shadow:
               f |= light_flag::type_hemi_shadow;
               break;
            case spot:
               f |= light_flag::type_spot;
               break;
            case spot_shadow:
               f |= light_flag::type_spot_shadow;
               break;
         }
         DATA.write(f);
      }
      DATA.write(this->falloff_exponent);
      DATA.write(this->fov);
      DATA.write(this->near_clip);
      DATA.write(this->flicker.period);
      DATA.write(this->flicker.amplitudes.intensity);
      DATA.write(this->flicker.amplitudes.movement);
      DATA.write(this->item_data.value);
      DATA.write(this->item_data.weight);
      DATA.close();
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->fade);
      FNAM.close();
      record.write_formID_subrecord('SNAM', this->item_data.sound, true);
      return true;
   }
   void Light::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->model.clear(*this);
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      //
      this->item_data.sound.set(*this, nullptr);
      this->item_data = decltype(item_data)();
      //
      this->flicker = decltype(flicker)();
      this->fade    = 1.0F;
      this->time    = 0.0F;
      this->radius  = 128;
      this->color   = { 255, 255, 255, 255 };
      this->falloff_exponent = 1.0;
      this->fov = 90;
      this->near_clip = 0;
      //
      this->light_flags = 0;
      this->light_type  = engine_light_type::omni;
      

   }
   void Light::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      //
      this->item_data.sound.clear_if(*this, other);
   }
}