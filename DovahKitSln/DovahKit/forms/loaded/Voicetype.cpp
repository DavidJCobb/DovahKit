#include "Voicetype.h"
#include "../../esp/LoadOrder.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   void Voicetype::load(TESPluginRecord& record) {
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'DNAM': // flags
               subrecord.read(this->flags);
               break;
         }
      }
   }
   /*static*/ void Voicetype::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
      return; // this form type does not have any subrecords that contain form IDs
   }
}