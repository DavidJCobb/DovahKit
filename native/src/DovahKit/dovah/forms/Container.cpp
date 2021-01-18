#include "Container.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Container::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
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
            case 'OBND':
               this->bounds.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord, intfc);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               this->inventory.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->container_flags);
               subrecord.read(this->weight);
               break;
            case 'SNAM': // open sound
               subrecord.read(this->open_sound);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, *this->stub, this->open_sound)
               );
               break;
            case 'QNAM': // close sound
               subrecord.read(this->close_sound);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::sound_descriptor, *this->stub, this->close_sound)
               );
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void Container::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'SNAM': // open sound
            case 'QNAM': // close sound
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generate_use_info(subrecord, uib);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               components::container_data::generate_use_info(subrecord, uib);
               break;
            case 'OBND': // bounds
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'DATA': // flags and weight
               break;
         }
      }
   }
   void Container::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->inventory.sever_outbound_references_to(other, *this);
      //
      this->open_sound.clear_if(*this, other);
      this->close_sound.clear_if(*this, other);
   }
   void Container::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->model.clear(*this);
      this->bounds.clear();
      this->name.reset();
      this->inventory.clear(*this);
      this->container_flags = 0;
      this->weight = 0.0F;
      this->open_sound.set(*this, nullptr);
      this->close_sound.set(*this, nullptr);
   }
}