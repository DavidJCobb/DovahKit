#include "WordOfPower.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void WordOfPower::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'FULL':
               subrecord.to_string(this->dragon_name);
               break;
            case 'TNAM':
               subrecord.to_string(this->human_name);
               break;
            default:
               intfc.log_load_warning(
                  file_read_warning::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void WordOfPower::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   bool WordOfPower::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<WordOfPower*>(out);
      if (!copy)
         return false;
      copy->dragon_name = this->dragon_name;
      copy->human_name  = this->human_name;
      return true;
   }
   bool WordOfPower::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->dragon_name);
      FULL.close();
      auto& TNAM = record.open_next_subrecord('TNAM');
      TNAM.write(this->human_name);
      TNAM.close();
      return true;
   }
}