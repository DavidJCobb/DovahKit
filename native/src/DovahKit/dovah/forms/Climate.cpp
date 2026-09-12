#include "Climate.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Climate::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'WLST':
               {
                  auto& item = this->weather_types.emplace_back();
                  if (auto& form = item.weather; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::weather, subrecord.signature());
                  subrecord.read(item.chance);
                  if (auto& form = item.global; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::global, subrecord.signature());
               }
               break;
            case 'FNAM':
               subrecord.read(this->textures.sun);
               break;
            case 'GNAM':
               subrecord.read(this->textures.sun_glare);
               break;
            case 'MODL':
            case 'MODT':
            case 'MOSD':
               this->night_sky_nif.load(subrecord, intfc);
               break;
            case 'TNAM':
               subrecord.read(this->timing.sunrise.begin.value);
               subrecord.read(this->timing.sunrise.end.value);
               subrecord.read(this->timing.sunset.begin.value);
               subrecord.read(this->timing.sunset.end.value);
               subrecord.read(this->timing.volatility);
               subrecord.read(this->timing.moon_phase.value);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Climate::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      struct weather_type_use_info {
         form_id_t weather;
         form_id_t global;
      };
      std::vector<weather_type_use_info> weather_types;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'WLST':
               {
                  auto& item = weather_types.emplace_back();
                  subrecord.read(item.weather);
                  subrecord.skip_bytes(sizeof(weather_type::chance));
                  subrecord.read(item.global);
               }
               break;
         }
      }
      for (auto& item : weather_types) {
         uib.add_outbound_reference(item.weather);
         uib.add_outbound_reference(item.global);
      }
   }
   void Climate::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Climate*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->night_sky_nif.clone_from(this->night_sky_nif);
      copy->textures = this->textures;
      copy->timing = this->timing;
      {
         auto& src_list = this->weather_types;
         auto& dst_list = copy->weather_types;
         for (auto& item : dst_list) {
            item.global.set(*copy, nullptr);
            item.weather.set(*copy, nullptr);
         }
         dst_list.clear();

         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            dst_list[i].global.set(*copy, src_list[i].global);
            dst_list[i].weather.set(*copy, src_list[i].weather);
            dst_list[i].chance = src_list[i].chance;
         }
      }
   }
   void Climate::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      for (auto& item : this->weather_types) {
         auto& subrecord = record.open_next_subrecord('WLST');
         subrecord.reserve_more(0x0C);
         subrecord.write(item.weather);
         subrecord.write(item.chance);
         subrecord.write(item.global);
         subrecord.close();
      }
      record.write_string_subrecord('FNAM', this->textures.sun);
      record.write_string_subrecord('GNAM', this->textures.sun_glare);
      this->night_sky_nif.save(record, intfc, 'MODL', 'MODT');
      {
         auto& subrecord = record.open_next_subrecord('TNAM');
         subrecord.reserve_more(0x06);
         subrecord.write(this->timing.sunrise.begin.value);
         subrecord.write(this->timing.sunrise.end.value);
         subrecord.write(this->timing.sunset.begin.value);
         subrecord.write(this->timing.sunset.end.value);
         subrecord.write(this->timing.volatility);
         subrecord.write(this->timing.moon_phase.value);
         subrecord.close();
      }
   }
   void Climate::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->night_sky_nif.clear();
      this->textures = {};
      this->timing = {};
      
      for (auto& item : this->weather_types) {
         item.global.set(*this, nullptr);
         item.weather.set(*this, nullptr);
      }
      this->weather_types.clear();
   }
   void Climate::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& item : this->weather_types) {
         item.global.clear_if(*this, other);
         item.weather.clear_if(*this, other);
      }
   }
}