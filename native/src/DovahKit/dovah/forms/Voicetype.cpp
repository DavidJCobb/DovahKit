#include "Voicetype.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Voicetype::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DNAM': // flags
               subrecord.read(this->voicetype_flags);
               break;
            default:
               intfc.log_load_warning(
                  file_read_warning::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
               );
               break;
         }
      }
   }
   /*static*/ void Voicetype::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
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