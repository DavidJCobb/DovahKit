#include "Color.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Color::load(tes_record_reader& record) {
      Form::load(record);
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'CNAM':
               this->color.load(subrecord);
               break;
            case 'FNAM':
               subrecord.read(this->color_flags);
               break;
         }
      }
   }
   /*static*/ void Color::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   bool Color::_save_impl(tes_record_writer& record) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& CNAM = record.open_next_subrecord('CNAM');
      this->color.save(CNAM);
      CNAM.close();
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->color_flags);
      FNAM.close();
      return true;
   }
}