#include "LoadingScreen.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void LoadingScreen::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'NNAM':
               if (auto& form = this->static_model; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::statik, subrecord.signature());
               break;
            case 'SNAM':
               subrecord.read(this->initial_coords.scale);
               break;
            case 'RNAM':
               subrecord.read(this->initial_coords.rotation.x);
               subrecord.read(this->initial_coords.rotation.y);
               subrecord.read(this->initial_coords.rotation.z);
               break;
            case 'ONAM':
               subrecord.read(this->rotation_constraints.min);
               subrecord.read(this->rotation_constraints.max);
               break;
            case 'XNAM':
               subrecord.read(this->initial_coords.translation.x);
               subrecord.read(this->initial_coords.translation.y);
               subrecord.read(this->initial_coords.translation.z);
               break;
            case 'MOD2':
               subrecord.read(this->camera_path);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void LoadingScreen::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t static_model;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'NNAM':
               subrecord.read(static_model);
               break;
         }
      }
      uib.add_outbound_reference(static_model);
   }
   void LoadingScreen::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (LoadingScreen*)out;

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->description = this->description;

      copy->camera_path          = this->camera_path;
      copy->initial_coords       = this->initial_coords;
      copy->rotation_constraints = this->rotation_constraints;

      copy->static_model.set(*copy, this->static_model);
   }
   void LoadingScreen::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      record.write_formID_subrecord('NNAM', this->static_model);
      {
         auto& subrecord = record.open_next_subrecord('SNAM');
         subrecord.write(this->initial_coords.scale);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('RNAM');
         subrecord.write(this->initial_coords.rotation.x);
         subrecord.write(this->initial_coords.rotation.y);
         subrecord.write(this->initial_coords.rotation.z);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('ONAM');
         subrecord.write(this->rotation_constraints.min);
         subrecord.write(this->rotation_constraints.max);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('XNAM');
         subrecord.write(this->initial_coords.translation.x);
         subrecord.write(this->initial_coords.translation.y);
         subrecord.write(this->initial_coords.translation.z);
         subrecord.close();
      }
      record.write_string_subrecord('MOD2', this->camera_path);
   }
   void LoadingScreen::_clear_impl() noexcept {
      this->conditions.clear(*this);
      this->script_data.clear(*this);

      this->description.reset();

      this->camera_path.clear();
      this->initial_coords = {};
      this->rotation_constraints = {};
      this->static_model.set(*this, nullptr);
   }
   void LoadingScreen::_sever_outbound_references_impl(form_stub& other) noexcept {
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->static_model.clear_if(*this, other);
   }
}