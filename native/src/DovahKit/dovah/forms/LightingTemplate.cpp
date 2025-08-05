#include "LightingTemplate.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void LightingTemplate::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'DATA':
               this->ambient.base_color.load(subrecord);
               this->directional.color.load(subrecord);
               this->fog.colors.near.load(subrecord);
               subrecord.read(this->fog.near);
               subrecord.read(this->fog.far);
               subrecord.read(this->directional.rotation.xy);
               subrecord.read(this->directional.rotation.z);
               subrecord.read(this->directional.fade);
               subrecord.read(this->fog.clip_distance);
               subrecord.read(this->fog.power);
               subrecord.skip_bytes(32); // this would be the ambient colors (everything in DALC), but for whatever reason, this form prefers to store those elsewhere
               this->fog.colors.far.load(subrecord);
               subrecord.read(this->fog.max);
               subrecord.read(this->light_fade_distance.start);
               subrecord.read(this->light_fade_distance.end);
               if (record.version() >= 34) {
                  subrecord.skip_bytes(4); // "inherit flags"
               }
               break;
            case 'DALC':
               this->ambient.x.positive.load(subrecord);
               this->ambient.x.negative.load(subrecord);
               this->ambient.y.positive.load(subrecord);
               this->ambient.y.negative.load(subrecord);
               this->ambient.z.positive.load(subrecord);
               this->ambient.z.negative.load(subrecord);
               this->ambient.specular.load(subrecord);
               subrecord.read(this->ambient.fresnel);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void LightingTemplate::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void LightingTemplate::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (LightingTemplate*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->ambient = this->ambient;
      copy->directional = this->directional;
      copy->fog = this->fog;
      copy->light_fade_distance = this->light_fade_distance;
   }
   void LightingTemplate::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         this->ambient.base_color.save(subrecord);
         this->directional.color.save(subrecord);
         this->fog.colors.near.save(subrecord);
         subrecord.write(this->fog.near);
         subrecord.write(this->fog.far);
         subrecord.write(this->directional.rotation.xy);
         subrecord.write(this->directional.rotation.z);
         subrecord.write(this->directional.fade);
         subrecord.write(this->fog.clip_distance);
         subrecord.write(this->fog.power);
         subrecord.skip_bytes(32); // this would be the ambient colors (everything in DALC), but for whatever reason, this form prefers to store those elsewhere
         this->fog.colors.far.save(subrecord);
         subrecord.write(this->fog.max);
         subrecord.write(this->light_fade_distance.start);
         subrecord.write(this->light_fade_distance.end);
         if (record.version() >= 34) {
            subrecord.skip_bytes(4); // "inherit flags"
         }
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DALC');
         this->ambient.x.positive.save(subrecord);
         this->ambient.x.negative.save(subrecord);
         this->ambient.y.positive.save(subrecord);
         this->ambient.y.negative.save(subrecord);
         this->ambient.z.positive.save(subrecord);
         this->ambient.z.negative.save(subrecord);
         this->ambient.specular.save(subrecord);
         subrecord.write(this->ambient.fresnel);
         subrecord.close();
      }
   }
   void LightingTemplate::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->ambient = {};
      this->directional.color = {};
      this->directional.fade = 1;
      this->directional.rotation = {};
      this->fog = {};
      this->light_fade_distance = {};
   }
   void LightingTemplate::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}