#include "AssociationType.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void AssociationType::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'MPRT': // "male parent"
               subrecord.read(this->referrer.masc);
               break;
            case 'FPRT': // "female parent"
               subrecord.read(this->referrer.fem);
               break;
            case 'MCHT': // "male child"
               subrecord.read(this->referent.masc);
               break;
            case 'FCHT': // "female child"
               subrecord.read(this->referent.fem);
               break;
            case 'DATA':
               subrecord.read(this->flags);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void AssociationType::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      return; // this form type does not have any subrecords that contain form IDs
   }
   void AssociationType::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (AssociationType*)out;
      
      copy->referrer = this->referrer;
      copy->referent = this->referent;
      copy->flags    = this->flags;
   }
   void AssociationType::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      record.write_string_subrecord('MPRT', this->referrer.masc);
      record.write_string_subrecord('FPRT', this->referrer.fem);
      record.write_string_subrecord('MCHT', this->referent.masc);
      record.write_string_subrecord('FCHT', this->referent.fem);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->flags);
      DATA.close();
   }
   void AssociationType::_clear_impl() noexcept {
      this->referent = {};
      this->referrer = {};
      this->flags = 0;
   }
}