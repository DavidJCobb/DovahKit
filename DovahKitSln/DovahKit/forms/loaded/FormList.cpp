#include "FormList.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   void FormList::load(TESPluginRecord& record) {
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
   /*static*/ void FormList::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
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