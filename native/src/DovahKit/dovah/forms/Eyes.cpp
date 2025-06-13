#include "Eyes.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Eyes::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'OBND':
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'ICON':
               subrecord.read(this->texture);
               break;
            case 'DATA':
               subrecord.read(this->flags);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Eyes::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
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
   void Eyes::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Eyes*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name = this->name;

      copy->flags = this->flags;
      copy->texture = this->texture;
   }
   void Eyes::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      if (!this->texture.empty())
         record.write_string_subrecord('ICON', this->texture);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->flags);
         subrecord.close();
      }
   }
   void Eyes::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->name.reset();
      this->flags = 0;
      this->texture.clear();
   }
   void Eyes::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}