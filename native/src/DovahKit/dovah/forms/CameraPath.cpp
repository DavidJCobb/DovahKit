#include "CameraPath.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void CameraPath::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;

            case 'ANAM':
               if (auto& form = this->parent; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::camera_path, subrecord.signature());
               if (auto& form = this->previous_sibling; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::camera_path, subrecord.signature());
               break;
            case 'DATA':
               {
                  uint8_t value = 0;
                  subrecord.read(value);
                  if (value & 0x80) {
                     this->zoom.must_have_camera_shots = false;
                     value &= ~0x80;
                  } else {
                     this->zoom.must_have_camera_shots = true;
                  }
                  this->zoom.type = (zoom_type)value;
               }
               break;
            case 'SNAM':
               if (auto& form = this->camera_shots.emplace_back(); subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::camera_shot, subrecord.signature());
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void CameraPath::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      form_id_t parent;
      form_id_t previous_sibling;
      std::vector<form_id_t> camera_shots;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'ANAM':
               subrecord.read(parent);
               subrecord.read(previous_sibling);
               break;
            case 'SNAM':
               subrecord.read(camera_shots.emplace_back());
               break;
         }
      }
      uib.add_outbound_reference(parent);
      uib.add_outbound_reference(previous_sibling);
      for(auto id : camera_shots)
         uib.add_outbound_reference(id);
   }
   void CameraPath::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (CameraPath*)out;

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      copy->script_data.clone_from(this->script_data, *copy);

      copy->parent.set(*copy, this->parent);
      copy->previous_sibling.set(*copy, this->previous_sibling);
      copy->zoom = this->zoom;
      copy_form_reference_list(*copy, copy->camera_shots, this->camera_shots);
   }
   void CameraPath::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      for (auto& cnd : this->conditions) {
         cnd.save(record, intfc);
      }
      {
         auto& subrecord = record.open_next_subrecord('ANAM');
         subrecord.write(this->parent);
         subrecord.write(this->previous_sibling);
         subrecord.close();
      }
      {
         uint8_t packed = (uint8_t)this->zoom.type;
         if (!this->zoom.must_have_camera_shots) {
            packed |= 0x80;
         }
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(packed);
         subrecord.close();
      }
      for (auto& form : this->camera_shots)
         record.write_formID_subrecord('SNAM', form);
   }
   void CameraPath::_clear_impl() noexcept {
      this->conditions.clear(*this);
      this->script_data.clear(*this);

      this->parent.set(*this, nullptr);
      this->previous_sibling.set(*this, nullptr);
      this->zoom = {};
      clear_form_reference_list(this->camera_shots, *this);
   }
   void CameraPath::_sever_outbound_references_impl(form_stub& other) noexcept {
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->parent.clear_if(*this, other);
      this->previous_sibling.clear_if(*this, other);
      remove_form_from_reference_list(this->camera_shots, other, *this);
   }
}