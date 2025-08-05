#include "ConstructibleObject.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ConstructibleObject::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               this->inventory.load(subrecord, intfc);
               break;
            case 'CNAM':
               if (auto& form = this->result.form; subrecord.read(form))
                  ; // TODO: Are there any constraints on the type allowed here?
               break;
            case 'BNAM':
               if (auto& form = this->workbench_keyword; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::keyword, subrecord.signature());
               break;
            case 'NAM1':
               subrecord.read(this->result.count);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void ConstructibleObject::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t result_form;
      form_id_t workbench_keyword;

      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               components::container_data::generate_use_info(subrecord, uib);
               break;
            case 'CNAM': // open sound
               subrecord.read(result_form);
               break;
            case 'BNAM': // close sound
               subrecord.read(workbench_keyword);
               break;
         }
      }
      uib.add_outbound_reference(result_form);
      uib.add_outbound_reference(workbench_keyword);
   }
   void ConstructibleObject::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ConstructibleObject*)out;

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      copy->inventory.clone_from(this->inventory, *copy);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->result.form.set(*copy, this->result.form);
      copy->result.count = this->result.count;
      copy->workbench_keyword.set(*copy, this->workbench_keyword);
   }
   void ConstructibleObject::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->inventory.save(record, intfc);
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      record.write_formID_subrecord('CNAM', this->result.form);
      record.write_formID_subrecord('BNAM', this->workbench_keyword);
      {
         auto& subrecord = record.open_next_subrecord('NAM1');
         subrecord.write(this->result.count);
         subrecord.close();
      }
   }
   void ConstructibleObject::_clear_impl() noexcept {
      this->conditions.clear(*this);
      this->inventory.clear(*this);
      this->script_data.clear(*this);

      this->result.count = 1;
      this->result.form.set(*this, nullptr);
      this->workbench_keyword.set(*this, nullptr);
   }
   void ConstructibleObject::_sever_outbound_references_impl(form_stub& other) noexcept {
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->inventory.sever_outbound_references_to(other, *this);

      this->result.form.clear_if(*this, other);
      this->workbench_keyword.clear_if(*this, other);
   }
}