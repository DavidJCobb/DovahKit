#include "IdleMarker.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void IdleMarker::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      uint8_t expected_anim_count = 0;
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
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case components::idle_collection::subrecord_signature_array:
            case components::idle_collection::subrecord_signature_count:
            case components::idle_collection::subrecord_signature_flags:
            case components::idle_collection::subrecord_signature_timer:
               this->data.load(subrecord, intfc);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void IdleMarker::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      
      components::idle_collection::use_info_state idle_collection;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case components::idle_collection::subrecord_signature_array:
            case components::idle_collection::subrecord_signature_count:
            case components::idle_collection::subrecord_signature_flags:
            case components::idle_collection::subrecord_signature_timer:
               idle_collection.read(subrecord);
               break;
         }
      }
      idle_collection.commit(uib);
   }
   void IdleMarker::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (IdleMarker*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->bounds = this->bounds;
      copy->model.clone_from(this->model, *copy);
      copy->data.clone_from(this->data, *copy);
   }
   void IdleMarker::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      this->data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT', 'MODS');
   }
   void IdleMarker::_clear_impl() noexcept {
      this->bounds.clear();
      this->model.clear(*this);
      this->script_data.clear(*this);
      this->data.clear(*this);
   }
   void IdleMarker::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->data.sever_outbound_references_to(other, *this);
   }
}