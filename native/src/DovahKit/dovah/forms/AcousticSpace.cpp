#include "AcousticSpace.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void AcousticSpace::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
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
            case 'SNAM':
               if (auto& dst = this->ambient_sound; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::sound_descriptor, subrecord.signature());
               break;
            case 'RDAT':
               if (auto& dst = this->region; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::region, subrecord.signature());
               break;
            case 'BNAM':
               if (auto& dst = this->environment_type; subrecord.read(dst))
                  intfc.warn_if_ref_is_wrong_type(dst, form_type::reverb_parameters, subrecord.signature());
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void AcousticSpace::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t ambient_sound;
      form_id_t environment_type;
      form_id_t region;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'BNAM':
               subrecord.read(environment_type);
               break;
            case 'SNAM':
               subrecord.read(ambient_sound);
               break;
            case 'RDAT':
               subrecord.read(region);
               break;
         }
      }
      uib.add_outbound_reference(ambient_sound);
      uib.add_outbound_reference(environment_type);
      uib.add_outbound_reference(region);
   }
   void AcousticSpace::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (AcousticSpace*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->ambient_sound.set(*copy, this->ambient_sound);
      copy->environment_type.set(*copy, this->environment_type);
      copy->region.set(*copy, this->region);
   }
   void AcousticSpace::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      record.write_formID_subrecord('SNAM', this->ambient_sound,    true);
      record.write_formID_subrecord('RDAT', this->region,           true);
      record.write_formID_subrecord('BNAM', this->environment_type, true);
   }
   void AcousticSpace::_clear_impl() noexcept {
      this->bounds.clear();
      this->script_data.clear(*this);
      this->ambient_sound.set(*this, nullptr);
      this->environment_type.set(*this, nullptr);
      this->region.set(*this, nullptr);
   }
   void AcousticSpace::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->ambient_sound.clear_if(*this, other);
      this->environment_type.clear_if(*this, other);
      this->region.clear_if(*this, other);
   }
}