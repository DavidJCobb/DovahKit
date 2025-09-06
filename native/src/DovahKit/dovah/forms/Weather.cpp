#include "Weather.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/weather/extra_directional_ambient_lighting_params.h"
#include "../notices/form_load_warnings/by_form_type/weather/too_much_layer_data.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::weather;
   }
}

namespace dovah::loaded_forms {
   void Weather::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      uint32_t current_layer_alpha  = 0;
      uint32_t current_layer_color  = 0;
      uint32_t current_dalc         = 0;
      uint32_t layers_with_textures = 0;
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

            #pragma region Legacy cloud texture paths
               case 'DNAM':
                  subrecord.read(this->clouds.layers[0].texture);
                  layers_with_textures |= 1 << 0;
                  break;
               case 'CNAM':
                  subrecord.read(this->clouds.layers[1].texture);
                  layers_with_textures |= 1 << 1;
                  break;
               case 'BNAM':
                  subrecord.read(this->clouds.layers[2].texture);
                  layers_with_textures |= 1 << 2;
                  break;
               case 'ANAM':
                  subrecord.read(this->clouds.layers[3].texture);
                  layers_with_textures |= 1 << 3;
                  break;
               case 'ENAM':
                  subrecord.read(this->clouds.layers[4].texture);
                  layers_with_textures |= 1 << 4;
                  break;
            #pragma endregion
            case 'LNAM':
               subrecord.read(this->max_cloud_layers);
               break;
            case 'MNAM':
               if (auto& form = this->precipitation.form; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::shader_particle_geometry_data, subrecord.signature());
               break;
            case 'NNAM':
               if (auto& form = this->visual_effect.form; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::visual_effect, subrecord.signature());
               break;
            case 'ONAM': // legacy RNAM
               for(size_t i = 0; i < this->clouds.layers.size(); ++i) {
                  auto& v = this->clouds.layers[i].speed.y.raw;
                  subrecord.read(v);
                  v = (v >> 1) + 0x7F;
               }
               break;
            case 'QNAM':
               for (auto& layer : this->clouds.layers)
                  subrecord.read(layer.speed.x.raw);
               break;
            case 'RNAM':
               for (auto& layer : this->clouds.layers)
                  subrecord.read(layer.speed.y.raw);
               break;
            case 'PNAM':
               if (current_layer_color < max_cloud_layer_count) {
                  auto& layer = this->clouds.layers[current_layer_color];
                  layer.time_of_day.sunrise.color.load(subrecord);
                  layer.time_of_day.day.color.load(subrecord);
                  layer.time_of_day.sunset.color.load(subrecord);
                  layer.time_of_day.night.color.load(subrecord);
               }
               ++current_layer_color;
               break;
            case 'JNAM':
               if (current_layer_alpha < max_cloud_layer_count) {
                  auto& layer = this->clouds.layers[current_layer_color];
                  subrecord.read(layer.time_of_day.sunrise.alpha);
                  subrecord.read(layer.time_of_day.day.alpha);
                  subrecord.read(layer.time_of_day.sunset.alpha);
                  subrecord.read(layer.time_of_day.night.alpha);
               }
               ++current_layer_alpha;
               break;
            case 'NAM0':
               for (auto& color : this->colors.list) {
                  color.sunrise.load(subrecord);
                  color.day.load(subrecord);
                  color.sunset.load(subrecord);
                  color.night.load(subrecord);
               }
               break;
            case 'FNAM':
               subrecord.read(this->fog_distance.day.near);
               subrecord.read(this->fog_distance.day.far);
               subrecord.read(this->fog_distance.night.near);
               subrecord.read(this->fog_distance.night.far);
               subrecord.read(this->fog_distance.day.power);
               subrecord.read(this->fog_distance.night.power);
               subrecord.read(this->fog_distance.day.max);
               subrecord.read(this->fog_distance.night.max);
               break;
            case 'DATA':
               subrecord.read(this->wind.speed);
               subrecord.skip_bytes(2);
               subrecord.read(this->trans_delta);
               subrecord.read(this->sun.glare);
               subrecord.read(this->sun.damage);
               subrecord.read(this->precipitation.transition.intro.raw);
               subrecord.read(this->precipitation.transition.outro.raw);
               subrecord.read(this->thunderstorm.transition.intro.raw);
               subrecord.read(this->thunderstorm.transition.outro.raw);
               subrecord.read(this->thunderstorm.frequency);
               subrecord.read(this->flags);
               subrecord.read(this->thunderstorm.lightning_color.r);
               subrecord.read(this->thunderstorm.lightning_color.g);
               subrecord.read(this->thunderstorm.lightning_color.b);
               subrecord.read(this->visual_effect.transition.intro.raw);
               subrecord.read(this->visual_effect.transition.outro.raw);
               subrecord.read(this->wind.direction.base);
               subrecord.read(this->wind.direction.variance);
               break;
            case 'NAM1':
               subrecord.read(this->clouds.disabled_layers);
               break;
            case 'SNAM':
               {
                  auto& sound = this->sounds.emplace_back();
                  if (auto& form = sound.form; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, std::array{ form_type::sound_descriptor, form_type::sound }, subrecord.signature());
                  subrecord.read(sound.type);
               }
               break;
            case 'TNAM':
               if (auto& form = this->sky_statics.emplace_back(); subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::statik, subrecord.signature());
               break;
            case 'IMSP':
               if (auto& form = this->imagespaces.sunrise; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::imagespace, subrecord.signature());
               if (auto& form = this->imagespaces.day; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::imagespace, subrecord.signature());
               if (auto& form = this->imagespaces.sunset; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::imagespace, subrecord.signature());
               if (auto& form = this->imagespaces.night; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::imagespace, subrecord.signature());
               break;
            case 'HNAM':
               if (record.is_skyrim_special()) {
                  auto& opt = this->volumetric;
                  if (!opt.has_value())
                     opt.emplace();
                  auto& val = opt.value();
                  if (auto& form = val.sunrise; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::volumetric_lighting, subrecord.signature());
                  if (auto& form = val.day; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::volumetric_lighting, subrecord.signature());
                  if (auto& form = val.sunset; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::volumetric_lighting, subrecord.signature());
                  if (auto& form = val.night; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::volumetric_lighting, subrecord.signature());
               } else {
                  intfc.warn_on_unrecognized_subrecord(subrecord);
               }
               break;
            case 'DALC':
               if (current_dalc < this->directional_ambient_lighting.list.size()) {
                  this->directional_ambient_lighting.list[current_dalc].load(subrecord, intfc);
               }
               ++current_dalc;
               break;
            case 'NAM2':
               this->colors.moon_glare.sunrise.load(subrecord);
               this->colors.moon_glare.day.load(subrecord);
               this->colors.moon_glare.sunset.load(subrecord);
               this->colors.moon_glare.night.load(subrecord);
               break;
            case 'NAM3':
               this->colors.sun_glare.sunrise.load(subrecord);
               this->colors.sun_glare.day.load(subrecord);
               this->colors.sun_glare.sunset.load(subrecord);
               this->colors.sun_glare.night.load(subrecord);
               break;
            case 'MODL':
            case 'MODT':
            case 'MOSD':
               this->aurora.load(subrecord, intfc);
               break;
            case 'GNAM':
               if (record.is_skyrim_special()) {
                  if (auto& form = this->sun.lens_flare; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::lens_flare, subrecord.signature());
               } else {
                  intfc.warn_on_unrecognized_subrecord(subrecord);
               }
               break;
            default:
               {
                  uint32_t index = (signature - '00TX') >> 24;
                  if (index < max_cloud_layer_count) {
                     subrecord.read(this->clouds.layers[index].texture);
                     layers_with_textures |= (1 << index);
                     break;
                  }
               }
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      this->clouds.disabled_layers |= ~layers_with_textures;

      if (current_layer_alpha > max_cloud_layer_count) {
         specific_load_warnings::too_much_layer_data notice(
            this->stub,
            specific_load_warnings::too_much_layer_data::data_type::alpha,
            current_layer_alpha
         );
         intfc.log_load_warning(notice);
      }
      if (current_layer_color > max_cloud_layer_count) {
         specific_load_warnings::too_much_layer_data notice(
            this->stub,
            specific_load_warnings::too_much_layer_data::data_type::color,
            current_layer_alpha
         );
         intfc.log_load_warning(notice);
      }
      if (current_dalc > this->directional_ambient_lighting.list.size()) {
         specific_load_warnings::extra_directional_ambient_lighting_params notice(
            this->stub,
            current_dalc
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void Weather::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      struct {
         form_id_t sunrise;
         form_id_t day;
         form_id_t sunset;
         form_id_t night;
      } imagespaces;
      form_id_t precipitation;
      std::vector<form_id_t> sky_statics;
      std::vector<form_id_t> sounds;
      struct {
         form_id_t lens_flare;
      } sun;
      form_id_t visual_effect;
      struct {
         form_id_t sunrise;
         form_id_t day;
         form_id_t sunset;
         form_id_t night;
      } volumetric;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'MNAM':
               subrecord.read(precipitation);
               break;
            case 'NNAM':
               subrecord.read(visual_effect);
               break;
            case 'SNAM':
               subrecord.read(sounds.emplace_back());
               break;
            case 'TNAM':
               subrecord.read(sky_statics.emplace_back());
               break;
            case 'IMSP':
               subrecord.read(imagespaces.sunrise);
               subrecord.read(imagespaces.day);
               subrecord.read(imagespaces.sunset);
               subrecord.read(imagespaces.night);
               break;
            case 'HNAM':
               if (!record.is_skyrim_special())
                  break;
               subrecord.read(volumetric.sunrise);
               subrecord.read(volumetric.day);
               subrecord.read(volumetric.sunset);
               subrecord.read(volumetric.night);
               break;
            case 'GNAM':
               if (!record.is_skyrim_special())
                  break;
               subrecord.read(sun.lens_flare);
               break;
         }
      }
      uib.add_outbound_reference(imagespaces.sunrise);
      uib.add_outbound_reference(imagespaces.day);
      uib.add_outbound_reference(imagespaces.sunset);
      uib.add_outbound_reference(imagespaces.night);
      uib.add_outbound_reference(precipitation);
      for (auto id : sky_statics)
         uib.add_outbound_reference(id);
      for (auto id : sounds)
         uib.add_outbound_reference(id);
      uib.add_outbound_reference(sun.lens_flare);
      uib.add_outbound_reference(visual_effect);
      uib.add_outbound_reference(volumetric.sunrise);
      uib.add_outbound_reference(volumetric.day);
      uib.add_outbound_reference(volumetric.sunset);
      uib.add_outbound_reference(volumetric.night);
   }
   void Weather::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Weather*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->max_cloud_layers = this->max_cloud_layers;
      copy->aurora.clone_from(this->aurora);
      copy->clouds = this->clouds;
      copy->colors = this->colors;
      copy->directional_ambient_lighting = this->directional_ambient_lighting;
      copy->fog_distance = this->fog_distance;
      {
         auto& src = this->imagespaces;
         auto& dst = copy->imagespaces;
         dst.sunrise.set(*copy, src.sunrise);
         dst.day.set(*copy, src.day);
         dst.sunset.set(*copy, src.sunset);
         dst.night.set(*copy, src.night);
      }
      copy_form_reference_list(*copy, copy->sky_statics, this->sky_statics);
      {
         auto& src_list = this->sounds;
         auto& dst_list = copy->sounds;
         for (auto& item : dst_list)
            item.form.set(*copy, nullptr);
         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst_list[i].form.set(*copy, src_list[i].form);
            dst_list[i].type = src_list[i].type;
         }
      }
      if (this->volumetric.has_value()) {
         if (!copy->volumetric.has_value())
            copy->volumetric.emplace();
         auto& src = this->volumetric.value();
         auto& dst = copy->volumetric.value();
         dst.sunrise.set(*copy, src.sunrise);
         dst.day.set(*copy, src.day);
         dst.sunset.set(*copy, src.sunset);
         dst.night.set(*copy, src.night);
      } else {
         if (copy->volumetric.has_value()) {
            auto& dst = copy->volumetric.value();
            dst.sunrise.set(*copy, nullptr);
            dst.day.set(*copy, nullptr);
            dst.sunset.set(*copy, nullptr);
            dst.night.set(*copy, nullptr);
            copy->volumetric.reset();
         }
      }

      copy->flags = this->flags;
      copy->precipitation.form.set(*copy, this->precipitation.form);
      copy->precipitation.transition = this->precipitation.transition;
      copy->sun.lens_flare.set(*copy, this->sun.lens_flare);
      copy->sun.glare  = this->sun.glare;
      copy->sun.damage = this->sun.damage;
      copy->thunderstorm = this->thunderstorm;
      copy->visual_effect.form.set(*copy, this->visual_effect.form);
      copy->visual_effect.transition = this->visual_effect.transition;
      copy->wind = this->wind;
      copy->trans_delta = this->trans_delta;
   }
   void Weather::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      for (size_t i = 0; i < this->clouds.layers.size(); ++i) {
         auto& layer = this->clouds.layers[i];
         auto& path  = layer.texture;
         if (path.empty())
            continue;
         record.write_string_subrecord('00TX' + i, path);
      }
      {
         auto& subrecord = record.open_next_subrecord('LNAM');
         subrecord.write(this->max_cloud_layers);
         subrecord.close();
      }
      record.write_formID_subrecord('MNAM', this->precipitation.form);
      record.write_formID_subrecord('NNAM', this->visual_effect.form);
      {
         auto& subrecord = record.open_next_subrecord('RNAM');
         for (auto& layer : this->clouds.layers)
            subrecord.write(layer.speed.y.raw);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('QNAM');
         for (auto& layer : this->clouds.layers)
            subrecord.write(layer.speed.x.raw);
         subrecord.close();
      }
      for (auto& layer : this->clouds.layers) {
         auto& subrecord = record.open_next_subrecord('PNAM');
         layer.time_of_day.sunrise.color.save(subrecord);
         layer.time_of_day.day.color.save(subrecord);
         layer.time_of_day.sunset.color.save(subrecord);
         layer.time_of_day.night.color.save(subrecord);
         subrecord.close();
      }
      for (auto& layer : this->clouds.layers) {
         auto& subrecord = record.open_next_subrecord('JNAM');
         subrecord.write(layer.time_of_day.sunrise.alpha);
         subrecord.write(layer.time_of_day.day.alpha);
         subrecord.write(layer.time_of_day.sunset.alpha);
         subrecord.write(layer.time_of_day.night.alpha);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('NAM0');
         for (auto& color : this->colors.list) {
            color.sunrise.save(subrecord);
            color.day.save(subrecord);
            color.sunset.save(subrecord);
            color.night.save(subrecord);
         }
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('FNAM');
         subrecord.write(this->fog_distance.day.near);
         subrecord.write(this->fog_distance.day.far);
         subrecord.write(this->fog_distance.night.near);
         subrecord.write(this->fog_distance.night.far);
         subrecord.write(this->fog_distance.day.power);
         subrecord.write(this->fog_distance.night.power);
         subrecord.write(this->fog_distance.day.max);
         subrecord.write(this->fog_distance.night.max);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->wind.speed);
         subrecord.skip_bytes(2);
         subrecord.write(this->trans_delta);
         subrecord.write(this->sun.glare);
         subrecord.write(this->sun.damage);
         subrecord.write(this->precipitation.transition.intro.raw);
         subrecord.write(this->precipitation.transition.outro.raw);
         subrecord.write(this->thunderstorm.transition.intro.raw);
         subrecord.write(this->thunderstorm.transition.outro.raw);
         subrecord.write(this->thunderstorm.frequency);
         subrecord.write(this->flags);
         subrecord.write(this->thunderstorm.lightning_color.r);
         subrecord.write(this->thunderstorm.lightning_color.g);
         subrecord.write(this->thunderstorm.lightning_color.b);
         subrecord.write(this->visual_effect.transition.intro.raw);
         subrecord.write(this->visual_effect.transition.outro.raw);
         subrecord.write(this->wind.direction.base);
         subrecord.write(this->wind.direction.variance);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('NAM1');
         subrecord.write(this->clouds.disabled_layers);
         subrecord.close();
      }
      for (auto& sound : this->sounds) {
         auto& subrecord = record.open_next_subrecord('SNAM');
         subrecord.write(sound.form);
         subrecord.write(sound.type);
         subrecord.close();
      }
      for (auto& form : this->sky_statics)
         record.write_formID_subrecord('TNAM', form, true);
      {
         auto& subrecord = record.open_next_subrecord('IMSP');
         subrecord.write(this->imagespaces.sunrise);
         subrecord.write(this->imagespaces.day);
         subrecord.write(this->imagespaces.sunset);
         subrecord.write(this->imagespaces.night);
         subrecord.close();
      }
      if (record.is_skyrim_special() && this->volumetric.has_value()) {
         auto& subrecord = record.open_next_subrecord('GNAM');
         auto& data      = this->volumetric.value();
         subrecord.write(data.sunrise);
         subrecord.write(data.day);
         subrecord.write(data.sunset);
         subrecord.write(data.night);
         subrecord.close();
      } else {
         if (this->volumetric.has_value()) {
            auto& data = this->volumetric.value();
            data.sunrise.set(*this, nullptr);
            data.day.set(*this, nullptr);
            data.sunset.set(*this, nullptr);
            data.night.set(*this, nullptr);
         }
      }
      for (auto& item : this->directional_ambient_lighting.list) {
         auto& subrecord = record.open_next_subrecord('DALC');
         item.save(subrecord, intfc);
         subrecord.close();
      }
      {  // Aurora
         if (!this->aurora.model_path.empty()) {
            auto& subrecord = record.open_next_subrecord('MODL');
            this->aurora.save_model_path(subrecord, intfc);
            subrecord.close();
         }
         if (this->aurora.has_precached_info()) {
            auto& subrecord = record.open_next_subrecord('MODT');
            this->aurora.save_precached_info(subrecord, intfc);
            subrecord.close();
         }
      }
      if (record.is_skyrim_special()) {
         record.write_formID_subrecord('GNAM', this->sun.lens_flare, true);
      } else {
         this->sun.lens_flare.set(*this, nullptr);
      }
   }
   void Weather::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->max_cloud_layers = 29;
      this->clouds.disabled_layers = 0xFFFFFFFF;
      this->clouds.layers = {};

      this->colors = {};
      this->directional_ambient_lighting = {};
      this->fog_distance = {};
      {
         this->imagespaces.sunrise.set(*this, nullptr);
         this->imagespaces.day.set(*this, nullptr);
         this->imagespaces.sunset.set(*this, nullptr);
         this->imagespaces.night.set(*this, nullptr);
      }
      clear_form_reference_list(this->sky_statics, *this);
      {
         auto& list = this->sounds;
         for (auto& item : list)
            item.form.set(*this, nullptr);
         list.clear();
      }
      {
         auto& opt = this->volumetric;
         if (opt.has_value()) {
            auto& v = opt.value();
            v.sunrise.set(*this, nullptr);
            v.day.set(*this, nullptr);
            v.sunset.set(*this, nullptr);
            v.night.set(*this, nullptr);
         }
      }

      this->flags = {};
      this->precipitation.form.set(*this, nullptr);
      this->sun.lens_flare.set(*this, nullptr);
      this->thunderstorm = {};
      this->visual_effect.form.set(*this, nullptr);
      this->wind = {};
      this->trans_delta = 0;
   }
   void Weather::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      {
         this->imagespaces.sunrise.clear_if(*this, other);
         this->imagespaces.day.clear_if(*this, other);
         this->imagespaces.sunset.clear_if(*this, other);
         this->imagespaces.night.clear_if(*this, other);
      }
      remove_form_from_reference_list(this->sky_statics, other, *this);
      {
         auto& list    = this->sounds;
         bool  changed = false;
         for (auto& item : list) {
            item.form.clear_if(*this, other);
            if (!item.form)
               changed = true;
         }
         if (changed)
            std::erase_if(list, [](auto& item) { return !item.form; });
      }
      {
         auto& opt = this->volumetric;
         if (opt.has_value()) {
            auto& v = opt.value();
            v.sunrise.clear_if(*this, other);
            v.day.clear_if(*this, other);
            v.sunset.clear_if(*this, other);
            v.night.clear_if(*this, other);

            if (!v.sunrise && !v.day && !v.sunset && !v.night)
               opt.reset();
         }
      }
      this->precipitation.form.clear_if(*this, other);
      this->sun.lens_flare.clear_if(*this, other);
      this->visual_effect.form.clear_if(*this, other);
   }
}