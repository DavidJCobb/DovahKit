#include "MagicEffect.h"
#include "../../esp/LoadOrder.h"
#include "../../esp/TESPlugin.h"
#include "../conditions.h"

namespace LoadedForms {
   /*static*/ void MagicEffect::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
      form_id_t formID;
      uint32_t  keywordCount = 0;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               PapyrusScriptData::generateUseInfo(subrecord, stub);
               break;
            case 'MDOB': // menu display object
            case 'ESCE': // counter effects
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
            case 'DATA':
               subrecord.skip_bytes(8); // flags, base cost
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(12); // skill, resistance, unknown
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(4); // taper weight
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(32); // skill level, area, casting time, taper curve, taper duration, second AV weight, effect type, primary AV
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(12);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(4); // skill usage mult
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(4); // dual cast scale
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(8);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               subrecord.skip_bytes(12);
               break;
            case 'SNDD':
               subrecord.skip_bytes(4);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'CTDA':
               Condition::generateUseInfo(record, stub);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'DNAM': // description
               break;
         }
      }
   }
}