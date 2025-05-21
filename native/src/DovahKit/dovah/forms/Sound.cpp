#include "Sound.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Sound::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'SDSC':
               if (auto& form = this->descriptor; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Sound::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t descriptor;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND':
               break;
            case 'SDSC':
               subrecord.read(descriptor);
               break;
         }
      }
      uib.add_outbound_reference(descriptor);
   }
   void Sound::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Sound*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->descriptor.set(*copy, this->descriptor);
   }
   void Sound::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord('OBND');
      this->bounds.save(OBND, intfc);
      OBND.close();
      record.write_formID_subrecord('SDSC', this->descriptor);
   }
   void Sound::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->bounds.clear();
      this->descriptor.set(*this, nullptr);
   }
   void Sound::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->descriptor.clear_if(*this, other);
   }
}