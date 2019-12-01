#include "Shout.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   void Shout::load(TESPluginRecord& record) {
      this->treatAsPower = (record.flags() & 0x00000080) != 0;
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'DESC':
               subrecord.to_string(this->description);
               break;
            case 'MDOB':
               subrecord.read(this->menuDisplayObjectID);
               break;
            case 'SNAM':
               {
                  Word& entry = this->words.emplace_back();
                  subrecord.read(entry.wordOfPowerID);
                  subrecord.read(entry.spellID);
                  subrecord.read(entry.recoveryTime);
               }
               break;
         }
      }
   }
   /*static*/ void Shout::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
      uint32_t keywordCount = 0;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'MDOB': // looping sound (e.g. nirnroot bell)
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'SNAM':
               if (subrecord.read(formID)) {
                  stub->add_outbound_reference(formID);
                  if (subrecord.read(formID)) {
                     stub->add_outbound_reference(formID);
                     // and then a four-byte float, which we can ignore
                  }
               }
               break;
            case 'EDID': // editor ID
            case 'FULL': // displayed name
            case 'DESC': // description
               break;
         }
      }
   }
}