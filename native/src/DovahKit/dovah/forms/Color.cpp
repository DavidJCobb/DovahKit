#include "Color.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Color::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'CNAM':
               this->color.load(subrecord);
               break;
            case 'FNAM':
               subrecord.read(this->color_flags);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Color::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   void Color::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Color*)out;
      
      copy->name  = this->name;
      copy->color = this->color;
      copy->color_flags = this->color_flags;
   }
   void Color::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& CNAM = record.open_next_subrecord('CNAM');
      this->color.save(CNAM);
      CNAM.close();
      auto& FNAM = record.open_next_subrecord('FNAM');
      FNAM.write(this->color_flags);
      FNAM.close();
   }
   void Color::_clear_impl() noexcept {
   }
}