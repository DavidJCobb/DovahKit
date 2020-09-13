#include "Activator.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   /*static*/ void Activator::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      uint32_t keywordCount = 0;
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generateUseInfo(subrecord, stub);
               break;
            case 'SNAM': // looping sound (e.g. nirnroot bell)
            case 'VNAM': // activation sound
            case 'WNAM': // water type, for water activators
            case 'KNAM': // interaction keyword
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generateUseInfo(subrecord, stub);
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
            case 'OBND': // bounds
               components::object_bounds::generateUseInfo(subrecord, stub);
               break;
            case 'EDID': // editor ID
            case 'FULL': // displayed name
            case 'PNAM': // marker color
            case 'RNAM': // override activation prompt text
            case 'FNAM': // extra flags
               break;
         }
      }
   }
}