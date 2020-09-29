#include "FormList.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void FormList::load(tes_record_reader& record) {
      Form::load(record);
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'LNAM': // list entry
               if (subrecord.read(formID))
                  this->contents.push_back(formID);
               break;
         }
      }
   }
   /*static*/ void FormList::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'LNAM': // list entry
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
         }
      }
   }
   bool FormList::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<FormList*>(out);
      if (!copy)
         return false;
      size_t size = this->contents.size();
      copy->contents.resize(size);
      for (size_t i = 0; i < size; ++i)
         copy->contents[i].set(copy->stub, this->contents[i]);
      return true;
   }
   bool FormList::_save_impl(tes_record_writer& record) {
      for (auto& entry : this->contents)
         record.write_formID_subrecord('LNAM', entry);
      return true;
   }
}