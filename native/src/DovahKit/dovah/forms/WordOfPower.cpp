#include "WordOfPower.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void WordOfPower::load(tes_record_reader& record) {
      Form::load(record);
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'FULL':
               subrecord.to_string(this->dragon_name);
               break;
            case 'TNAM':
               subrecord.to_string(this->human_name);
               break;
         }
      }
   }
   /*static*/ void WordOfPower::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   bool WordOfPower::_save_impl(tes_record_writer& record) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->dragon_name);
      FULL.close();
      auto& TNAM = record.open_next_subrecord('TNAM');
      TNAM.write(this->human_name);
      TNAM.close();
      return true;
   }
}