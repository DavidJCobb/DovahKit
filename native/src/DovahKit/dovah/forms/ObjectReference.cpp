#include "ObjectReference.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ObjectReference::load(tes_record_reader& record) {
      Form::load(record);
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.read(this->position.x);
               subrecord.read(this->position.y);
               subrecord.read(this->position.z);
               subrecord.read(this->rotation.x);
               subrecord.read(this->rotation.y);
               subrecord.read(this->rotation.z);
               break;
            case 'ONAM':
               this->is_open = true;
               break;
            case 'NAME': // base form (subrecord signature is vestigial from Morrowind, which used editor IDs instead of form IDs)
               subrecord.read(this->base_form);
               break;
            case 'VMAD':
               this->script_data.load(subrecord);
               break;
            default:
               if (this->extra_data.load(record) == components::extra_data_load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
               }
               break;
         }
      }
   }
   /*static*/ void ObjectReference::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               decltype(script_data)::generateUseInfo(subrecord, stub);
               break;
            case 'NAME': // base form (subrecord signature is vestigial from Morrowind, which used editor IDs instead of form IDs)
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID, use_info_entry::flag::object_reference);
               break;
            case 'EDID': // editor ID
            case 'ONAM':
            case 'DATA':
               break;
            default:
               if (components::extra_data_list::generate_use_info(record, stub) == components::extra_data_load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
               }
               break;
         }
      }
   }
   bool ObjectReference::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<ObjectReference*>(out);
      if (!copy)
         return false;
      assert(copy->stub);
      auto& clone_stub = *copy->stub;
      copy->extra_data.clone_from(this->extra_data, clone_stub);
      copy->script_data.clone_from(this->script_data, clone_stub);
      copy->base_form.set(&clone_stub, this->base_form);
      copy->is_open = this->is_open;
      copy->position = this->position;
      copy->rotation = this->rotation;
      return true;
   }
   bool ObjectReference::_save_impl(tes_record_writer& record) {
      this->script_data.save(record);
      //
      auto& NAME = record.open_next_subrecord('NAME');
      NAME.write(this->base_form);
      NAME.close();
      //
      this->extra_data.save(record);
      //
      if (this->is_open) {
         record.open_next_subrecord('ONAM').close();
      }
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->position.x);
      DATA.write(this->position.y);
      DATA.write(this->position.z);
      DATA.write(this->rotation.x);
      DATA.write(this->rotation.y);
      DATA.write(this->rotation.z);
      DATA.close();
      //
      return true;
   }
   void ObjectReference::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this->stub);
      this->extra_data.sever_outbound_references_to(other, *this->stub);
      //
      if (this->base_form == other.formID)
         this->base_form.set(this->stub, nullptr);
   }
}