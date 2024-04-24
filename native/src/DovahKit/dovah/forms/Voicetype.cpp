#include "Voicetype.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Voicetype::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DNAM': // flags
               subrecord.read(this->voicetype_flags);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Voicetype::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   void Voicetype::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Voicetype*)out;
      
      copy->voicetype_flags = this->voicetype_flags;
   }
   void Voicetype::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& DNAM = record.open_next_subrecord('DNAM');
      DNAM.write(this->voicetype_flags);
      DNAM.close();
   }
   void Voicetype::_clear_impl() noexcept {
      this->voicetype_flags = 0;
   }
}