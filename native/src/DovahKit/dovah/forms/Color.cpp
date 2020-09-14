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
               if (subrecord.is_in_bounds(4)) {
                  subrecord.unchecked_read(this->color.r);
                  subrecord.unchecked_read(this->color.g);
                  subrecord.unchecked_read(this->color.b);
                  subrecord.unchecked_read(this->color.unused);
               }
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
}