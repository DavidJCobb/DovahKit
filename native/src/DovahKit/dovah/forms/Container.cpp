#include "Container.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   /*static*/ void Container::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generateUseInfo(subrecord, stub);
               break;
            case 'SNAM': // open sound
            case 'QNAM': // close sound
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'MODL':
            case 'MODT':
            case 'MODS':
               components::model::generateUseInfo(subrecord, stub);
               break;
            case 'COCT':
            case 'CNTO':
            case 'COED':
               components::container_data::generateUseInfo(subrecord, stub);
               break;
            case 'OBND': // bounds
               components::object_bounds::generateUseInfo(subrecord, stub);
               break;
            case 'EDID': // editor ID
            case 'FULL': // name
            case 'DATA': // flags and weight
               break;
         }
      }
   }
}