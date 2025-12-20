#include "ImagespaceModifier.h"
#include "_common_cpp.h"
#include <array>

namespace {
   //
   // Bethesda ran out of subrecord names, so they started writing raw indices into 
   // subrecords, e.g. \x00IAD, \x01IAD, \x02IAD, and so on. This list maps these 
   // indices to pointers-to-members of the given interpolation mult/add list pairs. 
   // (We could've avoided the need for this by storing the pairs in a bare array, 
   // and indexing into that; but in general, I prefer to have loaded form classes 
   // organize their members by meaning rather than by what's convenient for the 
   // serialization code.)
   //
   constexpr const auto interpolation_offsets = []() {
      using loaded_form_type = dovah::loaded_forms::ImagespaceModifier;
      return std::array{
         // [0x00, 0x07] and [0x40, 0x47]:
         offsetof(loaded_form_type, hdr.eye_adapt_speed),
         offsetof(loaded_form_type, hdr.bloom.blur_radius),
         offsetof(loaded_form_type, hdr.bloom.threshold),
         offsetof(loaded_form_type, hdr.bloom.scale),
         offsetof(loaded_form_type, hdr.target_luminescence.min),
         offsetof(loaded_form_type, hdr.target_luminescence.max),
         offsetof(loaded_form_type, hdr.sunlight_scale),
         offsetof(loaded_form_type, hdr.sky_scale),
         // [0x07, 0x10] and [0x47, 0x50]:
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 0,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 1,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 2,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 3,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 4,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 5,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 6,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 7,
         offsetof(loaded_form_type, unknown) + sizeof(loaded_form_type::unknown[0]) * 8,
         // [0x11, 0x14] and [0x51, 0x54]:
         offsetof(loaded_form_type, cinematic.saturation),
         offsetof(loaded_form_type, cinematic.brightness),
         offsetof(loaded_form_type, cinematic.contrast),
         offsetof(loaded_form_type, cinematic.unused),
      };
   }();

   constexpr const uint8_t interpolation_index_count    = interpolation_offsets.size();
   constexpr const uint8_t interpolation_index_base_add = 0x40;
}

namespace dovah::loaded_forms {
   void ImagespaceModifier::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      
      if (!intfc.is_winning_record)
         return;

      /*

         ImagespaceModifier forms don't store flat values for most imagespace settings. 
         Rather, the modifier has a duration, and the vast majority of modifiable values 
         are stored as keyframe lists.

         The DNAM subrecord stores a small number of fixed values, along with the number 
         of keyframes for each keyframed field. Then, each keyframe list has dedicated 
         subrecords. Note that I said "each keyframe list" and not "each keyframed field:" 
         some fields consist of a multiplier and an offset, and both of those get their 
         own keyframe list.

         Complicating matters somewhat is that Bethesda actually ran out of subrecords for 
         the keyframe lists... so they started just giving each keyframed field an index, 
         and synthesizing subrecord signatures from those indices: \x00IAD and onward for 
         the multiplier keyframe lists, and \x40IAD and onward for the offset keyframe 
         lists. So for example, "basic blur radius" uses BNAM for its one keyframe list, 
         but "HDR Bloom Scale" uses \x03IAD and \x43IAD for its multiplier and offset 
         keyframe lists.

      */

      auto _read_mult_add_counts = [](tes_subrecord_reader& subrecord, interpolated_mult_add& v) {
         uint32_t mult_count = 0;
         uint32_t add_count  = 0;
         subrecord.read(mult_count);
         subrecord.read(add_count);
         v.mult.resize(mult_count);
         v.add.resize(add_count);
      };
      auto _read_single_counts = []<typename T>(tes_subrecord_reader& subrecord, std::vector<keyframe<T>>& list) {
         uint32_t count = 0;
         subrecord.read(count);
         list.resize(count);
      };

      auto _read_interp_list = []<typename T>(tes_subrecord_reader& subrecord, std::vector<keyframe<T>>&list) {
         size_t i = 0;
         for (; i < list.size(); ++i) {
            if (!subrecord.read(list[i].time))
               break;
            if constexpr (std::is_same_v<T, color_t>) {
               if (!list[i].value.load(subrecord))
                  break;
            } else {
               if (!subrecord.read(list[i].value))
                  break;
            }
         }
         for (; i < list.size(); ++i)
            list[i] = {};
      };
      
      while (auto& subrecord = record.next_subrecord()) {
         const auto signature = subrecord.signature();
         if (Form::subrecord_is_handled_elsewhere(signature))
            continue;
         switch (signature) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;

            case 'DNAM':
               subrecord.read(this->flags);
               subrecord.read(this->duration);
               //
               // Most of the data after this is counts for the various interpolated fields.
               //
               #pragma region HDR
                  _read_mult_add_counts(subrecord, this->hdr.eye_adapt_speed);
                  _read_mult_add_counts(subrecord, this->hdr.bloom.blur_radius);
                  _read_mult_add_counts(subrecord, this->hdr.bloom.threshold);
                  _read_mult_add_counts(subrecord, this->hdr.bloom.scale);
                  _read_mult_add_counts(subrecord, this->hdr.target_luminescence.min);
                  _read_mult_add_counts(subrecord, this->hdr.target_luminescence.max);
                  _read_mult_add_counts(subrecord, this->hdr.sunlight_scale);
                  _read_mult_add_counts(subrecord, this->hdr.sky_scale);
               #pragma endregion
               for (auto& item : this->unknown)
                  _read_mult_add_counts(subrecord, item);
               #pragma region Cinematic
                  _read_mult_add_counts(subrecord, this->cinematic.saturation);
                  _read_mult_add_counts(subrecord, this->cinematic.brightness);
                  _read_mult_add_counts(subrecord, this->cinematic.contrast);
                  _read_mult_add_counts(subrecord, this->cinematic.unused);
               #pragma endregion
               _read_single_counts(subrecord, this->colors.tint);
               _read_single_counts(subrecord, this->blurs.basic.radius);
               _read_single_counts(subrecord, this->double_vision.strength);
               _read_single_counts(subrecord, this->blurs.radial.strength);
               _read_single_counts(subrecord, this->blurs.radial.ramp_up);
               _read_single_counts(subrecord, this->blurs.radial.start);
               subrecord.read(this->blurs.radial.flags);
               subrecord.read(this->blurs.radial.center.x);
               subrecord.read(this->blurs.radial.center.y);
               _read_single_counts(subrecord, this->depth_of_field.strength);
               _read_single_counts(subrecord, this->depth_of_field.distance);
               _read_single_counts(subrecord, this->depth_of_field.range);
               subrecord.read(this->depth_of_field.use_target);
               subrecord.read(this->depth_of_field.flags);
               subrecord.skip_bytes(2);
               _read_single_counts(subrecord, this->blurs.radial.ramp_down.value);
               _read_single_counts(subrecord, this->blurs.radial.ramp_down.start);
               _read_single_counts(subrecord, this->colors.fade);
               _read_single_counts(subrecord, this->blurs.motion.strength);
               break;
            case 'BNAM':
               _read_interp_list(subrecord, this->blurs.basic.radius);
               break;
            case 'VNAM':
               _read_interp_list(subrecord, this->double_vision.strength);
               break;
            case 'TNAM':
               _read_interp_list(subrecord, this->colors.tint);
               break;
            case 'NAM3':
               _read_interp_list(subrecord, this->colors.fade);
               break;
            case 'RNAM':
               _read_interp_list(subrecord, this->blurs.radial.strength);
               break;
            case 'SNAM':
               _read_interp_list(subrecord, this->blurs.radial.ramp_up);
               break;
            case 'UNAM':
               _read_interp_list(subrecord, this->blurs.radial.start);
               break;
            case 'NAM1':
               _read_interp_list(subrecord, this->blurs.radial.ramp_down.value);
               break;
            case 'NAM2':
               _read_interp_list(subrecord, this->blurs.radial.ramp_down.start);
               break;
            case 'WNAM':
               _read_interp_list(subrecord, this->depth_of_field.strength);
               break;
            case 'XNAM':
               _read_interp_list(subrecord, this->depth_of_field.distance);
               break;
            case 'YNAM':
               _read_interp_list(subrecord, this->depth_of_field.range);
               break;
            case 'NAM4':
               _read_interp_list(subrecord, this->blurs.motion.strength);
               break;
            default:
               if ((signature & '\x00IAD') == '\x00IAD') {
                  constexpr const uint8_t index_base_mult = 0x00;
                  constexpr const uint8_t index_base_add  = 0x40;

                  uint8_t index       = signature & 0xFF;
                  uint8_t based_index = index;
                  if (index >= interpolation_index_base_add) {
                     based_index -= interpolation_index_base_add;
                  }
                  if (based_index < interpolation_index_count) {
                     auto& subject = *(interpolated_mult_add*)((uint8_t*)this + interpolation_offsets[based_index]);
                     if (index >= interpolation_index_base_add) {
                        _read_interp_list(subrecord, subject.add);
                     } else {
                        _read_interp_list(subrecord, subject.mult);
                     }
                     break;
                  }
               }
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ImagespaceModifier::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void ImagespaceModifier::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ImagespaceModifier*)out;

      copy->script_data.clone_from(this->script_data, *copy);

      copy->duration = this->duration;
      copy->flags    = this->flags;

      copy->blurs = this->blurs;
      copy->cinematic = this->cinematic;
      copy->colors = this->colors;
      copy->depth_of_field = this->depth_of_field;
      copy->double_vision = this->double_vision;
      copy->hdr = this->hdr;
      copy->unknown = this->unknown;
   }
   void ImagespaceModifier::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DNAM');

         auto _write_mult_add_counts = [&subrecord](const interpolated_mult_add& v) {
            subrecord.write((uint32_t)v.mult.size());
            subrecord.write((uint32_t)v.add.size());
         };
         auto _write_single_count = [&subrecord]<typename T>(const std::vector<keyframe<T>>& v) {
            subrecord.write((uint32_t)v.size());
         };

         subrecord.write(this->flags);
         subrecord.write(this->duration);
         //
         // Most of the data after this is counts for the various interpolated fields.
         //
         #pragma region HDR
         _write_mult_add_counts(this->hdr.eye_adapt_speed);
         _write_mult_add_counts(this->hdr.bloom.blur_radius);
         _write_mult_add_counts(this->hdr.bloom.threshold);
         _write_mult_add_counts(this->hdr.bloom.scale);
         _write_mult_add_counts(this->hdr.target_luminescence.min);
         _write_mult_add_counts(this->hdr.target_luminescence.max);
         _write_mult_add_counts(this->hdr.sunlight_scale);
         _write_mult_add_counts(this->hdr.sky_scale);
         #pragma endregion
         for (auto& item : this->unknown)
            _write_mult_add_counts(item);
         #pragma region Cinematic
         _write_mult_add_counts(this->cinematic.saturation);
         _write_mult_add_counts(this->cinematic.brightness);
         _write_mult_add_counts(this->cinematic.contrast);
         _write_mult_add_counts(this->cinematic.unused);
         #pragma endregion
         _write_single_count(this->colors.tint);
         _write_single_count(this->blurs.basic.radius);
         _write_single_count(this->double_vision.strength);
         _write_single_count(this->blurs.radial.strength);
         _write_single_count(this->blurs.radial.ramp_up);
         _write_single_count(this->blurs.radial.start);
         subrecord.write(this->blurs.radial.flags);
         subrecord.write(this->blurs.radial.center.x);
         subrecord.write(this->blurs.radial.center.y);
         _write_single_count(this->depth_of_field.strength);
         _write_single_count(this->depth_of_field.distance);
         _write_single_count(this->depth_of_field.range);
         subrecord.write(this->depth_of_field.use_target);
         subrecord.write(this->depth_of_field.flags);
         subrecord.skip_bytes(2);
         _write_single_count(this->blurs.radial.ramp_down.value);
         _write_single_count(this->blurs.radial.ramp_down.start);
         _write_single_count(this->colors.fade);
         _write_single_count(this->blurs.motion.strength);

         subrecord.close();
      }

      auto _write_interp_list = [&record]<typename T>(uint32_t signature, std::vector<keyframe<T>>& list) {
         auto& subrecord = record.open_next_subrecord(signature);
         for (auto& item : list) {
            subrecord.write(item.time);
            if constexpr (std::is_same_v<T, color_t>) {
               item.value.save(subrecord);
            } else {
               subrecord.write(item.value);
            }
         }
         subrecord.close();
      };
      _write_interp_list('BNAM', this->blurs.basic.radius);
      _write_interp_list('VNAM', this->double_vision.strength);
      _write_interp_list('TNAM', this->colors.tint);
      _write_interp_list('NAM3', this->colors.fade);
      _write_interp_list('RNAM', this->blurs.radial.strength);
      _write_interp_list('SNAM', this->blurs.radial.ramp_up);
      _write_interp_list('UNAM', this->blurs.radial.start);
      _write_interp_list('NAM1', this->blurs.radial.ramp_down.value);
      _write_interp_list('NAM2', this->blurs.radial.ramp_down.start);
      _write_interp_list('WNAM', this->depth_of_field.strength);
      _write_interp_list('XNAM', this->depth_of_field.distance);
      _write_interp_list('YNAM', this->depth_of_field.range);
      _write_interp_list('NAM4', this->blurs.motion.strength);
      for (size_t i = 0; i < interpolation_index_count; ++i) {
         auto& subject = *(interpolated_mult_add*)((uint8_t*)this + interpolation_offsets[i]);
         _write_interp_list(interpolator_subrecord(i), subject.mult);
         _write_interp_list(interpolator_subrecord(i + interpolation_index_base_add), subject.add);
      }
      for (size_t i = 0; i < interpolation_index_count; ++i) {
         auto& subject = *(interpolated_mult_add*)((uint8_t*)this + interpolation_offsets[i]);
         _write_interp_list(interpolator_subrecord(i + interpolation_index_base_add), subject.add);
      }
   }
   void ImagespaceModifier::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->duration = 0;
      this->flags    = 0;

      this->blurs = {};
      this->cinematic = {};
      this->colors = {};
      this->depth_of_field = {};
      this->double_vision = {};
      this->hdr = {};
      this->unknown = {};
   }
   void ImagespaceModifier::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}