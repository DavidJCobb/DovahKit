#include "Container.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Container::load(tes_record_reader& record) {
      Form::load(record);
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord);
               break;
            case 'OBND':
               this->bounds.load(subrecord);
               break;
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               this->model.load(subrecord);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               this->inventory.load(subrecord);
               break;
            case 'DATA':
               subrecord.read(this->container_flags);
               subrecord.read(this->weight);
               break;
            case 'SNAM': // open sound
               subrecord.read(this->open_sound);
               break;
            case 'QNAM': // close sound
               subrecord.read(this->close_sound);
               break;
         }
      }
   }
   /*static*/ void Container::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generateUseInfo(subrecord, stub);
               break;
            case 'SNAM': // open sound
            case 'QNAM': // close sound
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generateUseInfo(subrecord, stub);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               components::container_data::generateUseInfo(subrecord, stub);
               break;
            case 'OBND': // bounds
               components::object_bounds::generateUseInfo(subrecord, stub);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'DATA': // flags and weight
               break;
         }
      }
   }
   void Container::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this->stub);
      this->model.sever_outbound_references_to(other, *this->stub);
      this->inventory.sever_outbound_references_to(other, *this->stub);
      //
      this->open_sound.clear_if(*this->stub, other);
      this->close_sound.clear_if(*this->stub, other);
   }
}