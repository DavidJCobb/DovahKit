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
   bool Voicetype::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<Voicetype*>(out);
      if (!copy)
         return false;
      copy->voicetype_flags = this->voicetype_flags;
      return true;
   }
   bool Voicetype::_save_impl(tes_record_writer& record) {
      auto& DNAM = record.open_next_subrecord('DNAM');
      DNAM.write(this->voicetype_flags);
      DNAM.close();
      return true;
   }
}