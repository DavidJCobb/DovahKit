#include "Activator.h"
#include "../../esp/LoadOrder.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   /*static*/ void Activator::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
      uint32_t keywordCount = 0;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               PapyrusScriptData::generateUseInfo(subrecord, stub);
               break;
            case 'SNAM': // looping sound (e.g. nirnroot bell)
            case 'VNAM': // activation sound
            case 'WNAM': // water type, for water activators
            case 'KNAM': // interaction keyword
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'KSIZ':
               subrecord.read(keywordCount);
               break;
            case 'KWDA':
               if (!keywordCount)
                  keywordCount = subrecord.size() / 4;
               for(uint32_t i = 0; i < keywordCount; i++)
                  if (subrecord.read(formID))
                     stub->add_outbound_reference(formID);
               break;
            case 'EDID': // editor ID
            case 'OBND': // object bounds
            case 'FULL': // displayed name
            case 'PNAM': // marker color
            case 'RNAM': // override activation prompt text
            case 'FNAM': // extra flags
               break;
         }
      }
   }
}