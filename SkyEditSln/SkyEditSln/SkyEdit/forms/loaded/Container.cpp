#include "Container.h"
#include "../../esp/LoadOrder.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   /*static*/ void Container::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               PapyrusScriptData::generateUseInfo(subrecord, stub);
               break;
            case 'SNAM': // open sound
            case 'QNAM': // close sound
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               FormModel::generateUseInfo(subrecord, stub);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               ContainerData::generateUseInfo(subrecord, stub);
               break;
            case 'EDID': // editor ID
            case 'OBND': // bounds
            case 'FULL': // name
            case 'DATA': // flags and weight
               break;
         }
      }
   }
}