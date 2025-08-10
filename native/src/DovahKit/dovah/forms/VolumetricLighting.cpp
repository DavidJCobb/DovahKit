#include "VolumetricLighting.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void VolumetricLighting::setup(const file_load_order& load_order) noexcept {
      //
      // ...So, uh, I don't actually know which INI file the various default values 
      // are stored in. If I ever find that information, I should load the INI settings 
      // and initialize the fields here.
      //
   }
   void VolumetricLighting::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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

            case 'CNAM':
               subrecord.read(this->intensity);
               break;
            case 'DNAM':
               subrecord.read(this->custom_color.contribution);
               break;
            case 'ENAM':
               subrecord.read(this->custom_color.color.r);
               break;
            case 'FNAM':
               subrecord.read(this->custom_color.color.g);
               break;
            case 'GNAM':
               subrecord.read(this->custom_color.color.b);
               break;
            case 'HNAM':
               subrecord.read(this->density.contribution);
               break;
            case 'INAM':
               subrecord.read(this->density.size);
               break;
            case 'JNAM':
               subrecord.read(this->density.speeds.wind);
               break;
            case 'KNAM':
               subrecord.read(this->density.speeds.falling);
               break;
            case 'LNAM':
               subrecord.read(this->phase_function.contribution);
               break;
            case 'MNAM':
               subrecord.read(this->phase_function.scattering);
               break;
            case 'NNAM':
               subrecord.read(this->sampling_repartition.range_factor);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void VolumetricLighting::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
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
   void VolumetricLighting::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (VolumetricLighting*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->intensity = this->intensity;
      copy->custom_color = this->custom_color;
      copy->density = this->density;
      copy->phase_function = this->phase_function;
      copy->sampling_repartition = this->sampling_repartition;
   }
   void VolumetricLighting::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);

      auto _save_float = [&record](uint32_t signature, float v) {
         auto& subrecord = record.open_next_subrecord(signature);
         subrecord.write(v);
         subrecord.close();
      };

      _save_float('CNAM', this->intensity);
      _save_float('DNAM', this->custom_color.contribution);
      _save_float('ENAM', this->custom_color.color.r);
      _save_float('FNAM', this->custom_color.color.g);
      _save_float('GNAM', this->custom_color.color.b);
      _save_float('HNAM', this->density.contribution);
      _save_float('INAM', this->density.size);
      _save_float('JNAM', this->density.speeds.wind);
      _save_float('KNAM', this->density.speeds.falling);
      _save_float('LNAM', this->phase_function.contribution);
      _save_float('MNAM', this->phase_function.scattering);
      _save_float('NNAM', this->sampling_repartition.range_factor);
   }
   void VolumetricLighting::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->setup(this->stub.get_owning_load_order());
   }
   void VolumetricLighting::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}