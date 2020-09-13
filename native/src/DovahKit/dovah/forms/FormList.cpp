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
}