#include "MaterialObject.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void MaterialObject::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'MODL':
            case 'MODT':
               this->model.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->directional_material.falloff.scale);
               subrecord.read(this->directional_material.falloff.bias);
               subrecord.read(this->directional_material.noise_uv_scale);
               subrecord.read(this->directional_material.material_uv_scale);
               subrecord.read(this->directional_material.projection_vector.x);
               subrecord.read(this->directional_material.projection_vector.y);
               subrecord.read(this->directional_material.projection_vector.z);
               subrecord.read(this->directional_material.normal_dampener);
               subrecord.read(this->directional_material.single_pass_color.r);
               subrecord.read(this->directional_material.single_pass_color.g);
               subrecord.read(this->directional_material.single_pass_color.b);
               subrecord.read(this->directional_material.flags);
               if (record.version() >= 44) {
                  subrecord.read(this->directional_material.flags_ex);
                  subrecord.skip_bytes(3);
               }
               break;
            case 'DNAM':
               {
                  auto& buffer = this->properties.emplace_back();
                  buffer.resize(subrecord.size());
                  subrecord.read(buffer.data(), buffer.size());
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void MaterialObject::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void MaterialObject::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (MaterialObject*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->model.clone_from(this->model);

      copy->directional_material = this->directional_material;
      copy->properties = this->properties;
   }
   void MaterialObject::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT');
      for (const auto& property : this->properties) {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(property.data(), property.size());
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->directional_material.falloff.scale);
         subrecord.write(this->directional_material.falloff.bias);
         subrecord.write(this->directional_material.noise_uv_scale);
         subrecord.write(this->directional_material.material_uv_scale);
         subrecord.write(this->directional_material.projection_vector.x);
         subrecord.write(this->directional_material.projection_vector.y);
         subrecord.write(this->directional_material.projection_vector.z);
         subrecord.write(this->directional_material.normal_dampener);
         subrecord.write(this->directional_material.single_pass_color.r);
         subrecord.write(this->directional_material.single_pass_color.g);
         subrecord.write(this->directional_material.single_pass_color.b);
         subrecord.write(this->directional_material.flags);
         if (record.version() >= 44) {
            subrecord.write(this->directional_material.flags_ex);
            subrecord.skip_bytes(3);
         }
         subrecord.close();
      }
   }
   void MaterialObject::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->model.clear();
      
      this->directional_material = {};
      this->properties.clear();
   }
   void MaterialObject::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}