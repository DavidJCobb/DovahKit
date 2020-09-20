#include "Voicetype.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Voicetype::load(tes_record_reader& record) {
      Form::load(record);
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DNAM': // flags
               subrecord.read(this->voicetype_flags);
               break;
         }
      }
   }
   /*static*/ void Voicetype::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   bool Voicetype::save(tes_record_writer& record) {
      Form::save(record);
      //
      auto& DNAM = record.open_next_subrecord('DNAM');
      DNAM.write(this->voicetype_flags);
      DNAM.close();
      return true;
   }
}