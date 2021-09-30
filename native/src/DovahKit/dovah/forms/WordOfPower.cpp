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
               subrecord.read(this->dragon_name);
               break;
            case 'TNAM':
               subrecord.read(this->human_name);
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void WordOfPower::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   bool WordOfPower::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (WordOfPower*)out;
      //
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
   void WordOfPower::_clear_impl() noexcept {
      this->dragon_name.reset();
      this->human_name.reset();
   }
}