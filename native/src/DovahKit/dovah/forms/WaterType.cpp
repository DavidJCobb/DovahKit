#include "WaterType.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void WaterType::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'NNAM':
               //
               // This was intended to be three null-terminated strings in a single 
               // subrecord. However, the texture paths were implemented as TESIcon, 
               // which reads the entire subrecord... so the first in-memory texture 
               // path would've swallowed the whole subrecord and then (given that 
               // it expected a null-terminated string) ignored the latter two of 
               // the three intended paths.
               // 
               // Betting this is why Bethesda replaced NNAM with NAM2+NAM3+NAM4 in 
               // a game update.
               //
               {
                  size_t i = 0;
                  char   c = 0;
                  while (subrecord.read(c)) {
                     if (c == '\0') {
                        ++i;
                        if (i >= 3)
                           break;
                     } else {
                        this->noise.layers[i].texture += c;
                     }
                  }
                  for (auto& layer : this->noise.layers) {
                     if (layer.texture.starts_with('\\')) {
                        layer.texture.erase(0, 1);
                     }
                  }
               }
               break;
            case 'ANAM':
               subrecord.read(this->opacity);
               break;
            case 'FNAM':
               subrecord.read(this->flags);
               break;
            case 'MNAM':
               subrecord.read(this->legacy.material_id);
               break;
            case 'TNAM':
               if (auto& form = this->material_type; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::material_type, subrecord.signature());
               break;
            case 'SNAM':
               if (auto& form = this->sound; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'XNAM':
               if (auto& form = this->spell_to_apply; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::spell, subrecord.signature());
               break;
            case 'INAM':
               if (auto& form = this->underwater_imagespace; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::imagespace, subrecord.signature());
               break;
            case 'DATA':
               subrecord.read(this->damage_per_second);
               break;
            case 'DNAM':
               subrecord.read(this->legacy.wind.velocity);
               subrecord.read(this->legacy.wind.direction);
               subrecord.read(this->legacy.wave.amplitude);
               subrecord.read(this->legacy.wave.frequency);
               subrecord.read(this->specular.sun.power);
               subrecord.read(this->water.reflectivity);
               subrecord.read(this->water.fresnel);
               subrecord.read(this->unknown.DNAM_1C);
               subrecord.read(this->fog.above_water.distance.near);
               subrecord.read(this->fog.above_water.distance.far);
               this->water.colors.shallow.load(subrecord);
               this->water.colors.deep.load(subrecord);
               this->water.colors.reflection.load(subrecord);
               subrecord.read(this->legacy.texture_blend);
               subrecord.skip_bytes(3);
               subrecord.read(this->legacy.rain.force);
               subrecord.read(this->legacy.rain.velocity);
               subrecord.read(this->legacy.rain.falloff);
               subrecord.read(this->legacy.rain.dampener);
               subrecord.read(this->displacement.starting_size);
               subrecord.read(this->displacement.force);
               subrecord.read(this->displacement.velocity);
               subrecord.read(this->displacement.falloff);
               subrecord.read(this->displacement.dampen);
               subrecord.read(this->unknown.DNAM_5C);
               subrecord.read(this->noise.falloff);
               for (auto& layer : this->noise.layers)
                  subrecord.read(layer.wind_direction);
               for (auto& layer : this->noise.layers)
                  subrecord.read(layer.wind_speed);
               subrecord.read(this->unknown.DNAM_7C);
               subrecord.read(this->unknown.DNAM_80);
               subrecord.read(this->fog.above_water.amount);
               subrecord.read(this->unknown.DNAM_88);
               subrecord.read(this->fog.under_water.amount);
               subrecord.read(this->fog.under_water.distance.near);
               subrecord.read(this->fog.under_water.distance.far);
               subrecord.read(this->water.refraction_magnitude);
               subrecord.read(this->specular.power);
               subrecord.read(this->unknown.DNAM_A0);
               subrecord.read(this->specular.radius);
               subrecord.read(this->specular.brightness);
               for (auto& layer : this->noise.layers)
                  subrecord.read(layer.uv_scale);
               for (auto& layer : this->noise.layers)
                  subrecord.read(layer.amplitude_scale);
               subrecord.read(this->water.reflection_magnitude);
               subrecord.read(this->specular.sun.sparkle_magnitude);
               subrecord.read(this->specular.sun.specular_magnitude);
               subrecord.read(this->depth.reflections);
               subrecord.read(this->depth.refraction);
               subrecord.read(this->depth.normals);
               subrecord.read(this->depth.specular);
               subrecord.read(this->specular.sun.sparkle_power);
               if (record.is_skyrim_special()) {
                  subrecord.read(this->noise.flowmap_scale);
               }
               break;
            case 'GNAM':
               if (auto& form = this->legacy.related_waters.day; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::water_type, subrecord.signature());
               if (auto& form = this->legacy.related_waters.night; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::water_type, subrecord.signature());
               if (auto& form = this->legacy.related_waters.underwater; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::water_type, subrecord.signature());
               break;
            case 'NAM0':
               subrecord.read(this->velocity.linear.x);
               subrecord.read(this->velocity.linear.y);
               subrecord.read(this->velocity.linear.z);
               break;
            case 'NAM1':
               subrecord.read(this->velocity.angular.x);
               subrecord.read(this->velocity.angular.y);
               subrecord.read(this->velocity.angular.z);
               break;
            case 'NAM2':
               subrecord.read(this->noise.layers[0].texture);
               break;
            case 'NAM3':
               subrecord.read(this->noise.layers[1].texture);
               break;
            case 'NAM4':
               subrecord.read(this->noise.layers[2].texture);
               break;
            case 'NAM5':
               if (record.is_skyrim_special()) {
                  subrecord.read(this->noise.flowmap_texture);
                  break;
               }
               [[fallthrough]];
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void WaterType::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      form_id_t material_type;
      form_id_t sound;
      form_id_t spell_to_apply;
      form_id_t underwater_imagespace;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'TNAM':
               subrecord.read(material_type);
               break;
            case 'SNAM':
               subrecord.read(sound);
               break;
            case 'XNAM':
               subrecord.read(spell_to_apply);
               break;
            case 'INAM':
               subrecord.read(underwater_imagespace);
               break;
         }
      }
      uib.add_outbound_reference(material_type);
      uib.add_outbound_reference(sound);
      uib.add_outbound_reference(spell_to_apply);
      uib.add_outbound_reference(underwater_imagespace);
   }
   void WaterType::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (WaterType*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      //
      copy->name = this->name;

      copy->legacy.related_waters.day.set(*copy, this->legacy.related_waters.day);
      copy->legacy.related_waters.night.set(*copy, this->legacy.related_waters.night);
      copy->legacy.related_waters.underwater.set(*copy, this->legacy.related_waters.underwater);
      copy->legacy.material_id = this->legacy.material_id;
      copy->legacy.rain = this->legacy.rain;
      copy->legacy.texture_blend = this->legacy.texture_blend;
      copy->legacy.wave = this->legacy.wave;
      copy->legacy.wind = this->legacy.wind;

      copy->damage_per_second = this->damage_per_second;
      copy->depth = this->depth;
      copy->displacement = this->displacement;
      copy->flags = this->flags;
      copy->fog = this->fog;
      copy->material_type.set(*copy, this->material_type);
      copy->noise = this->noise;
      copy->opacity = this->opacity;
      copy->sound.set(*copy, this->sound);
      copy->specular = this->specular;
      copy->spell_to_apply.set(*copy, this->spell_to_apply);
      copy->underwater_imagespace.set(*copy, this->underwater_imagespace);
      copy->unknown = this->unknown;
      copy->velocity = this->velocity;
      copy->water = this->water;
   }
   void WaterType::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      {
         auto& subrecord = record.open_next_subrecord('ANAM');
         subrecord.write(this->opacity);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('FNAM');
         subrecord.write(this->flags);
         subrecord.close();
      }
      if (!this->legacy.material_id.empty())
         record.write_string_subrecord('MNAM', this->legacy.material_id);
      record.write_formID_subrecord('TNAM', this->material_type, true);
      record.write_formID_subrecord('SNAM', this->sound, true);
      record.write_formID_subrecord('XNAM', this->spell_to_apply, true);
      record.write_formID_subrecord('INAM', this->underwater_imagespace, true);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->damage_per_second);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->legacy.wind.velocity);
         subrecord.write(this->legacy.wind.direction);
         subrecord.write(this->legacy.wave.amplitude);
         subrecord.write(this->legacy.wave.frequency);
         subrecord.write(this->specular.sun.power);
         subrecord.write(this->water.reflectivity);
         subrecord.write(this->water.fresnel);
         subrecord.write(this->unknown.DNAM_1C);
         subrecord.write(this->fog.above_water.distance.near);
         subrecord.write(this->fog.above_water.distance.far);
         this->water.colors.shallow.save(subrecord);
         this->water.colors.deep.save(subrecord);
         this->water.colors.reflection.save(subrecord);
         subrecord.write(this->legacy.texture_blend);
         subrecord.skip_bytes(3);
         subrecord.write(this->legacy.rain.force);
         subrecord.write(this->legacy.rain.velocity);
         subrecord.write(this->legacy.rain.falloff);
         subrecord.write(this->legacy.rain.dampener);
         subrecord.write(this->displacement.starting_size);
         subrecord.write(this->displacement.force);
         subrecord.write(this->displacement.velocity);
         subrecord.write(this->displacement.falloff);
         subrecord.write(this->displacement.dampen);
         subrecord.write(this->unknown.DNAM_5C);
         subrecord.write(this->noise.falloff);
         for (auto& layer : this->noise.layers)
            subrecord.write(layer.wind_direction);
         for (auto& layer : this->noise.layers)
            subrecord.write(layer.wind_speed);
         subrecord.write(this->unknown.DNAM_7C);
         subrecord.write(this->unknown.DNAM_80);
         subrecord.write(this->fog.above_water.amount);
         subrecord.write(this->unknown.DNAM_88);
         subrecord.write(this->fog.under_water.amount);
         subrecord.write(this->fog.under_water.distance.near);
         subrecord.write(this->fog.under_water.distance.far);
         subrecord.write(this->water.refraction_magnitude);
         subrecord.write(this->specular.power);
         subrecord.write(this->unknown.DNAM_A0);
         subrecord.write(this->specular.radius);
         subrecord.write(this->specular.brightness);
         for (auto& layer : this->noise.layers)
            subrecord.write(layer.uv_scale);
         for (auto& layer : this->noise.layers)
            subrecord.write(layer.amplitude_scale);
         subrecord.write(this->water.reflection_magnitude);
         subrecord.write(this->specular.sun.sparkle_magnitude);
         subrecord.write(this->specular.sun.specular_magnitude);
         subrecord.write(this->depth.reflections);
         subrecord.write(this->depth.refraction);
         subrecord.write(this->depth.normals);
         subrecord.write(this->depth.specular);
         subrecord.write(this->specular.sun.sparkle_power);
         if (record.is_skyrim_special()) {
            subrecord.write(this->noise.flowmap_scale);
         }
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('GNAM');
         subrecord.write(this->legacy.related_waters.day);
         subrecord.write(this->legacy.related_waters.night);
         subrecord.write(this->legacy.related_waters.underwater);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('NAM0');
         subrecord.write(this->velocity.linear.x);
         subrecord.write(this->velocity.linear.y);
         subrecord.write(this->velocity.linear.z);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('NAM1');
         subrecord.write(this->velocity.angular.x);
         subrecord.write(this->velocity.angular.y);
         subrecord.write(this->velocity.angular.z);
         subrecord.close();
      }
      for (size_t i = 0; i < this->noise.layers.size(); ++i) {
         auto& layer = this->noise.layers[i];

         uint32_t signature = 'NAM2' + (std::endian::native == std::endian::little ? i : (i << 24));
         record.write_string_subrecord(signature, layer.texture);
      }
      {
         auto& v = this->noise.flowmap_texture;
         if (record.is_skyrim_special()) {
            if (!v.empty()) {
               record.write_string_subrecord('NAM5', v);
            }
         } else {
            v.clear();
         }
      }
   }
   void WaterType::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->name.reset();

      this->legacy.related_waters.day.set(*this, nullptr);
      this->legacy.related_waters.night.set(*this, nullptr);
      this->legacy.related_waters.underwater.set(*this, nullptr);

      this->damage_per_second = 0;
      this->depth = {};
      this->displacement = {};
      this->flags = 0;
      this->fog = {};
      this->legacy.rain = {};
      this->legacy.texture_blend = 50;
      this->legacy.wave = {};
      this->legacy.wind = {};
      this->material_type.set(*this, nullptr);
      this->noise = {};
      this->opacity = 75;
      this->sound.set(*this, nullptr);
      this->specular = {};
      this->spell_to_apply.set(*this, nullptr);
      this->underwater_imagespace.set(*this, nullptr);
      this->unknown = {};
      this->velocity = {};
      this->water = {};
   }
   void WaterType::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      this->material_type.clear_if(*this, other);
      this->sound.clear_if(*this, other);
      this->spell_to_apply.clear_if(*this, other);
      this->underwater_imagespace.clear_if(*this, other);

      this->legacy.related_waters.day.clear_if(*this, other);
      this->legacy.related_waters.night.clear_if(*this, other);
      this->legacy.related_waters.underwater.clear_if(*this, other);
   }
}